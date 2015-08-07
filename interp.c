/*
The blue programming language ("blue")
Copyright (C) 2007-2008  Erik R Lechak

email: erik@lechak.info
web: www.lechak.info

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "interp.h"


/* CALLENV */
static CallEnv callenv_new(CallEnv parent, Link function){
    CallEnv self = NULL;
    if ( !  parent){
        self                 = malloc( sizeof(*self));        
        self->next           = NULL;                    
        stack_new(self->stack, LinkStack, 16) 
        self->stack_offset   = 0; /* this is a root callenv (no parent) so the stack starts at the beginning */
    }else{
        
        /* try to retrieve a callenv from the linked-store */
        if (parent->next){
            self = parent->next;
        }else{
            self = parent->next = malloc( sizeof(*self));
            self->next = NULL;
        }
        self->stack = parent->stack;
        self->stack_offset = stack_length( parent->stack);
    }

    CodeBlock cb   = function->value.codeblock;
    self->function = function;
    self->current  = cb->bytecode;
    self->module   = cb->parent->value.module;
    self->start    = self->module->bytecode;    
    
    self->Self         = NULL;
    self->This         = NULL;
    self->Arg          = NULL;
    self->local        = NULL;
    self->Def          = NULL;
    self->linenum      = 0;

    self->parent = parent;
    return self;
}

/* root function */
static CallEnv callenv_new_root(Link function, Link This, Link Arg){
    link_dup(function);
    CallEnv env = callenv_new(NULL, function);
    env->Self = function;
    if (This) env->This     = link_dup(This);
    if (This) env->Def      = link_dup(This);
    env->Arg      = Arg ? link_dup(Arg) : array_new(0);
    return env;
}

/* do */
static CallEnv callenv_new_doblock(CallEnv parent, Link function){
    CallEnv env      = callenv_new(parent, function);  
    env->local       = parent->local;
    env->Self        = parent->Self;
    env->This        = env->parent->This;
    env->Arg         = env->parent->Arg;
    env->Def         = link_dup(env->parent->Def);
    return env;
}

/* function */
static CallEnv callenv_new_function(CallEnv parent, Link function, Link This, Link Arg){
    CallEnv env     = callenv_new(parent, function);
    env->Self       = function;
    env->This       = This;
    env->Arg        = Arg;
    env->Def        = link_dup(This);
    return env;
}

static CallEnv callenv_free( CallEnv self){
    CallEnv parent = self->parent;

    /* if this is a funcion block free this args and locals */
    if ( self->function == self->Self){
        if ( self->This )    link_free(self->This);
        if ( self->Arg )     link_free(self->Arg);
        if ( self->Def)      link_free(self->Def);
        if ( self->local )   linkarray_free(self->local);    
    }
    
    link_free( self->function);
    
    /* if this is a root callenv clear the stack and all chained callenv */
    if ( ! parent ){
        stack_free(self->stack);        
        do{
            parent = self->next;
            free(self);            
        }while((self = parent));
    }
    
    return parent;
}

static inline size_t read_offset(CallEnv env){
    size_t delta = *((size_t*)(env->current));
    env->current+= sizeof(size_t);
    return delta;
}

static inline int compare(CallEnv env){
    Link b = stack_pop(env->stack);
    Link a = stack_pop(env->stack);
    int i  = object_compare(a,b);
    link_free(a);
    link_free(b);
    return i;
}

static inline void binary_op(CallEnv env,  Link (*bin_op)(Link, Link)){
    Link b = stack_pop(env->stack);
    Link a = stack_pop(env->stack);
    Link link = bin_op(a,b);
    link_free(a);
    link_free(b);
    stack_push(env->stack, link);
}

static inline void unary_op(CallEnv env,  Link (*un_op)(Link)){
    Link a = stack_pop(env->stack);
    Link link = un_op(a);
    stack_push(env->stack, link);
    link_free(a);
}


/* create a backtrace for the critical link  */
static inline void addBacktrace(Link crit, Link codeblock_link, int linenum){
    if (! crit) return;
    if (! is_critical(crit)) return;
    
    Module module = codeblock_link->value.codeblock->parent->value.module;
    
    Link message = NULL;
    
    if (linenum)
        message = create_string_str( string_new_formatted("[module:%s   line:%i]", module->name->data, linenum));
    else
        message = create_string_str( string_new_formatted("[module:%s]", module->name->data) );

    Link backtrace = getAttr(crit, Global->backtrace_key);
        
    if ( ! backtrace ){
        backtrace = array_new(0);
        //fprintf(stderr, "createing backtrace array %p\n", backtrace);
        addAttr(crit,backtrace,Global->backtrace_key);
    }        

    array_push(backtrace, message);
    link_free(backtrace);
}




Link interpret(Link codeblock_link ,  Link This, Link Arg){
    
    CallEnv env = callenv_new_root( codeblock_link, This, Arg);
    LinkStack stack = env->stack;
    
    
    Link b          = NULL;
    Link link       = NULL;
    Link parent     = NULL;
    Link child_key  = NULL;
    
    Link pusher     = NULL; // Anything in this variable gets pushed onto the stack
    Link trapped    = NULL; // This is the last critical caught by the trap operator
    
    int    delta = 0; // jump delta

    /* Main interpreter loop */
    while(1){

        switch( *(env->current++) ){

            case INTRO:
                env->current+=4;
                break;

            case ALLOC_MODULE:
                delta = read_offset(env);
                env->module->global_vars = linkarray_new(delta);
                break;
            
            case ALLOC_LOCAL:
                delta = read_offset(env);
                env->local = linkarray_new(delta);
                break;
                                
            case NO_OP:
                break;
            
            case RETURN: // if there is something on the env->stack stack pop it off and return it, if not return null link
                if (   stack_length(stack) - env->stack_offset ){
                    pusher = stack_pop(stack);
                }else{
                    pusher = object_create(Global->null_type);
                }
                goto done;

            case RAISE: // if there is something on the stack stack pop it off and return it, if not return null link
            
                if (  stack_length(stack) - env->stack_offset ){
                    pusher = create_critical(stack_pop(stack));
                }else{
                    pusher = create_critical(object_create(Global->null_type));
                }
                goto done;

            case PUSH_LEX:
                delta = read_offset(env);
                //fprintf(stderr , "push lex [%i]    %p\n", delta,env->function->value.codeblock->lexicals);
                pusher = link_dup(env->function->value.codeblock->lexicals->vars[delta]);
                break;
                
            case STORE_LEX:
                delta = read_offset(env);
                //fprintf(stderr , "storing lex [%i]    %p\n", delta,env->function->value.codeblock->lexicals);
                b = env->function->value.codeblock->lexicals->vars[delta];
            
                if (b){
                    if  ( (b->type == Global->function_type) && 
                          (b->value.codeblock->lexicals == env->function->value.codeblock->lexicals)
                        )  b->value.codeblock->lexical_cycles--;
                    link_free(b);
                }
                
                b = link_dup(stack_peek(stack));
                if  ( (b->type == Global->function_type) && 
                      (b->value.codeblock->lexicals == env->function->value.codeblock->lexicals)
                    )  b->value.codeblock->lexical_cycles++;
                
                env->function->value.codeblock->lexicals->vars[delta] = b;
                
                break;
            
            case DEF:
                if (env->Def) link_free(env->Def);
                env->Def = stack_pop(env->stack);
                pusher = link_dup(env->Def);
                break;
            
            case PUSH_DEF:
                if ( ! env->Def){
                    pusher = exception("NoDefObject",NULL, NULL);
                }
                pusher = link_dup(env->Def);
                break;
                
                
                
            case ALLOC_LEXICAL:
                delta = read_offset(env);
                lexicals_alloc( env->function, delta);
                //env->lexical_root = 1;
                break;
                
            case STORE_ARG:
                delta = read_offset(env);
                
                if (env->Arg->value.array->length  > delta){
                    retry_store_arg:
                    env->Arg->value.array->links[delta] =  link_dup( stack_peek(stack) );
                }else{
                    array_push(env->Arg, NULL);
                    goto retry_store_arg;
                } 
                break;
                
            case PUSH_ARG:
                delta = read_offset(env);
                if (env->Arg->value.array->length  > delta){
                    pusher = link_dup(  env->Arg->value.array->links[delta]);
                }else{
                    pusher = exception("ArgsIndexOutOfBounds", NULL, NULL);
                }
            
                break;
                
            case STORE_GVAR:
                delta = read_offset(env);
                if (env->module->global_vars->links[delta]){
                    link_free(env->module->global_vars->links[delta]);
                }
                env->module->global_vars->links[delta] = link_dup( stack_peek(stack) );
                break;

            case STORE_VAR:
                delta = read_offset(env);

                if (env->local->links[delta]){
                    link_free(env->local->links[delta]);
                }
            
                env->local->links[delta] = link_dup( stack_peek(stack) );
                break;

            case STORE_CHILD:
                link        = stack_pop(stack); // value
                child_key   = stack_pop(stack); // key to find the child
                parent      = stack_pop(stack); // parent to look in
                pusher = object_addChild(parent, link, child_key);
                goto STORE_COMMON;

            case STORE_ATTR:
                link           = stack_pop(stack); // value
                child_key      = stack_pop(stack); // key to find the child
                parent         = stack_pop(stack); // parent to look in
                pusher = addAttr(parent, link,child_key);
                goto STORE_COMMON;
                
            STORE_COMMON:
                if (! pusher){
                    pusher = exception("AssignError", NULL, NULL);
                    link_free(link);
                }
            
                link_free(child_key);
                link_free(parent);
                break;

            case LT:
                delta = compare(env);
                pusher = create_numberi( (delta < 0)  ? 1 : 0 );
                break;

            case GT:
                delta = compare(env);
                pusher = create_numberi( (delta > 0)  ? 1 : 0 );
                break;

            case EQ:
                delta = compare(env);
                pusher = create_numberi( (delta == 0)  ? 1 : 0 );
                break;

            case NE:
                delta = compare(env);
                pusher = create_numberi( (delta == 0)  ? 0 : 1 );
                break;

            case LE:
                delta = compare(env);
                pusher = create_numberi( (delta <= 0)  ? 1 : 0 );
                break;

            case GE:
                delta = compare(env);
                pusher = create_numberi( (delta >= 0)  ? 1 : 0 );
                break;
        
            case CMP:
                delta = compare(env);
                pusher = create_numberi( delta );
                break;
            
            case OR:
            case AND:
                break;

            case SUB:
                b = stack_pop(env->stack);
                link = stack_pop(env->stack);
            
                if ( (link->type == Global->number_type) && (link->type == b->type)){
                    pusher = create_number(link->value.number - b->value.number);
                    link_free(link);
                    link_free(b);
                    break;
                }            
            
                pusher = object_op_minus(link,b);
                link_free(link);
                link_free(b);
            
                break;
            
            case ADD:
                b = stack_pop(env->stack);
                link = stack_pop(env->stack);
            
                if ( (link->type == Global->number_type) && (link->type == b->type)){
                    pusher = create_number(link->value.number + b->value.number);
                    link_free(link);
                    link_free(b);
                    break;
                }            
            
                pusher = object_op_plus(link,b);
                link_free(link);
                link_free(b);
                
                break;
                        
            case DIV:
                binary_op(env , object_op_divide);
                break;
            
            case MULT:
                binary_op(env , object_op_multiply);
                break;
            
            case MOD:
                binary_op(env , object_op_modulus);
                break;
            
            case POW:
                binary_op(env , object_op_power);
                break;
            
            case NEG:
                unary_op(env, object_op_neg);
                break;
            
            case NOT:
                link = stack_pop(stack);
                pusher = create_numberi(  (object_is_true(link))  ?  0 : 1 );
                link_free(link);
                break;

            case TEST:
            case ELSE:
                break;

            case DO:
                delta = read_offset(env);
                link  = codeblock_literal2( env->function->value.codeblock->parent, delta  );
                if (env->function->value.codeblock->lexicals) {
                    lexicals_attach( env->function->value.codeblock->lexicals, link);
                }
                env = callenv_new_doblock(env,link);
                break;
            
            case PUSH_ARRAY:
                delta = read_offset(env);
                pusher =  array_new_subarray( stack , delta);
                break;
                
            case CALL:
                link   = stack_pop(stack);     // the arguments in an array
                b      = stack_pop(stack);     // the function that gets called
                parent = link_dup(env->This);  // caller
                goto CALL_COMMON;

            
            case CALL_ATTR:            
                link       = stack_pop(stack);    // arguments
                child_key  = stack_pop(stack);    // name of the function
                parent     = stack_pop(stack);    // caller
                b = getAttr(parent,child_key); // the function that gets called           
                link_free(child_key); // no longer need the attributes key
            
                if (! b) {
                    pusher = exception("AttrNotFound", NULL, NULL);
                    break;
                }
                goto CALL_COMMON;
                
            case CALL_CHILD:
                link       = stack_pop(stack);     // ARG
                child_key  = stack_pop(stack);
                parent     = stack_pop(stack);     // THIS
                b = object_getChild(parent,child_key);
                link_free(child_key);
                goto CALL_COMMON;

            CALL_COMMON:
                /* function type so we can call it inline */
                if (b->type == Global->function_type){
                    env = callenv_new_function(env, b, parent, link); // ce , func,this, arg

                /* Not a CodeBlock so we have to use its virtual call function */
                }else{
                    pusher = object_call(b,  parent, link);// function, caller, args
                    link_free(link);
                    if (parent) link_free(parent);
                    link_free(b);
                    if (! pusher) pusher = object_create(Global->null_type);
                }
                break;

            case DEL_CHILD:
                child_key      = stack_pop(stack); // key to find the child
                parent         = stack_pop(stack); // parent to look in

                /* delete child from container */
                pusher = object_delChild(parent,child_key);
                if (! pusher) pusher = exception("ChildNotFound", object_getString(child_key), NULL);
                        
                link_free(child_key);
                link_free(parent);
                break;                
                
            case DEL_ATTR:
                child_key      = stack_pop(stack); // key to find the child
                parent         = stack_pop(stack); // parent to look in

                /* delete attr from container */
                pusher = delAttr(parent,child_key);
                if (! pusher) pusher = exception("AttrNotFound", object_getString(child_key), NULL);

                link_free(child_key);
                link_free(parent);
                break;                
                
            case GET_CHILD:
                child_key      = stack_pop(stack); // key to find the child
                parent         = stack_pop(stack); // parent to look in

                pusher = object_getChild(parent, child_key);            
                if (! pusher) pusher = exception("ChildNotFound", object_getString(child_key), NULL);

                link_free(parent);
                link_free(child_key);
                break;

            case GET_ATTR:
                child_key      = stack_pop(stack); // key to find the child
                parent         = stack_pop(stack); // parent to look in

                pusher = getAttr(parent, child_key);
                if (! pusher) pusher = exception("AttrNotFound", object_getString(child_key), NULL);

                link_free(parent);
                link_free(child_key);
                break;

            case TRAP:
                break;

            case CLEAR:
                for ( delta = stack_length( stack ) ; delta > env->stack_offset ; delta--){
                    link_free( stack_pop(stack) );    
                }    
                break;
            
            case STOP:
                break;

            done:
            case END:
                for ( delta = stack_length( stack ) ; delta > env->stack_offset ; delta--){
                    link_free( stack_pop(stack) );    
                }    
                addBacktrace(pusher, env->function, env->linenum);
                env = callenv_free(env);

                if (! env) goto end;
                
                if (! pusher) pusher = object_create(Global->null_type);
                break;

            /* JUMPS */
            case JIT:  /* Jump if true */
                delta = read_offset(env);
                link = stack_peek(stack);
                if ( link->type->is_true(link) )  env->current = env->start+delta;
                break;

            case JIF:  /* Jump if false */
                delta = read_offset(env);
                link = stack_peek(stack);
                if ( ! link->type->is_true(link) )  env->current = env->start+delta;
                break;
                

            case JIF_POP:  /* Pop value then jump if value is false,  */
                delta = read_offset(env);
                link = stack_pop(stack);
                if ( ! link->type->is_true(link) )  env->current = env->start+delta;
                link_free(link);
                break;
                
            case JUMP:  /* Absolute jump */
                delta = read_offset(env);
                env->current = env->start + delta;
                break;

            case JINC: /* Jump If Not Critical */
                delta = read_offset(env);
                env->current = env->start+delta;
                break;
                
              jinc_critical:
                delta = read_offset(env);
            
                if (trapped) link_free(trapped);
                trapped = pusher->value.link;
                pusher->value.link = NULL;
                link_free(pusher);
                pusher = NULL;
                break;

            case PUSH_NULL:
                pusher = create_null();
                break;

            case PUSH_NUM:
                pusher = create_number( *((number_t *)env->current) );
                env->current+= sizeof(number_t);
                break;
            
            case PUSH_STR:
                delta = read_offset(env);
                pusher = create_string_literal( env->start + delta  );
                break;

            case PUSH_GVAR:
                delta = read_offset(env);
                pusher = link_dup(env->module->global_vars->links[delta]);
            
                if (! pusher){
                    pusher = exception("GlobalVariableUsedBeforeSet",NULL,NULL);
                }
                break;
            
            case PUSH_VAR:
                delta = read_offset(env);
             
                pusher = link_dup(env->local->links[delta]);
                
                if (! pusher){
                    pusher = exception("VariableUsedBeforeSet",NULL,NULL);;
                }
                break;
            
            case PUSH_BLOCK:
                delta = read_offset(env);
                pusher  = codeblock_literal2( env->function->value.codeblock->parent, delta  );
                if (env->function->value.codeblock->lexicals) {
                    lexicals_attach( env->function->value.codeblock->lexicals, pusher);
                }
                break;

            case PUSH_SYS:
                pusher = link_dup(Global->SYS_MODULE);
                break;
            
            case PUSH_MODULE:
                pusher = link_dup( env->function->value.codeblock->parent);
                break;
            
            case TYPEOF:
                link = stack_pop(stack);
                pusher = link_dup( link->type->type_link);
                link_free(link);
                break;
            
            case PUSH_THIS:
                pusher = link_dup(env->This);
                break;

            case PUSH_SELF:
                pusher = link_dup(env->Self);
                break;
            
            case PUSH_ARGS:
                pusher = link_dup(env->Arg);
                break;

            case PUSH_TRAPPED:
                pusher = trapped ? link_dup(trapped) : object_create(Global->null_type);
                break;
            
            case POP:
                link_free( stack_pop(stack) );
                break;

            case LINE:
                env->linenum = read_offset(env);
                break;

            default:
                fprintf(stderr," UNRECOGNIZED OPCODE ERROR %i\n", *(env->current));
                fprintf(stderr,"     near line %i\n", (int)(env->linenum));
                exit(1);
                break;
        }
                
        if (pusher) {
            
            if ( is_critical( pusher ) ){
                if ( *(env->current) == JINC)  goto jinc_critical;
                goto done;
            }
            
            stack_push(stack , pusher);
            pusher = NULL;
        }
        
    }

    end:

    if (trapped) link_free(trapped);
    return pusher;
}

