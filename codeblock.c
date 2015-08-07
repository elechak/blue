/*
The blue programming language ("blue")
Copyright (C) 2007  Erik R Lechak

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

#include "global.h"


/* Closure/Lexical */

void lexicals_alloc( Link function, int size ){
    CodeBlock cb = function->value.codeblock;
    Lexicals lex = cb->lexicals = malloc( sizeof ( *(cb->lexicals)) );
    
    lex->vars = malloc( sizeof( *(lex->vars) ) * size );
    stack_new( lex->functions , LinkStack, 8);
    
    int c;
    
    for( c=0; c<size; c++) lex->vars[c] = NULL;
    lex->length = size;
    stack_push( lex->functions, function );
}

void lexicals_attach( Lexicals lex, Link function ){
    int c;
    Link link;
    
    //~ fprintf(stderr , "attaching function %p to %p\n", function, lex);
    stack_iter( lex->functions, link, c){
        if ( link == function) return;
    }

    function->value.codeblock->lexicals=lex;
    stack_push( lex->functions, function ); 
    
    
    //~ stack_iter( lex->functions, link, c){
        //~ fprintf(stderr , "             attached: %p   %i\n", link, link->refcount);
    //~ }    
}


void lexicals_detach( Link function){
    Lexicals lex = function->value.codeblock->lexicals;

    if (! lex) return;
    int c;
    Link link;
    int length = stack_length( lex->functions );
    
    function->value.codeblock->lexicals=NULL;
    
    stack_iter( lex->functions, link, c){
        
        if ( link == function) {
            
            if ( c == (length-1)){
                link = stack_pop( lex->functions );
                
            }else{
                link = stack_pop( lex->functions );
                stack_get( lex->functions, c) = link;
            }

            return;            
        }
    }
}

void lexicals_free( Link function ){
    int c;
    Link link;
    Lexicals lex = function->value.codeblock->lexicals;
    
    if ( function->refcount == 0) lexicals_detach(function);
    
    /* look through referant functions and ensure that all existing ref-counts are cyclic */
    stack_iter( lex->functions, link, c){
        if (link->value.codeblock->lexical_cycles != link->refcount) return;
    }    

    /* if you got here there are no external links to these lexical varaibles so we can free them */
    
    /* clear the lexical refernce from all referants */
    stack_iter( lex->functions, link, c){
        link->value.codeblock->lexicals = NULL;
    }
    
    /* free all of the stored lexical variables */
    for( c=0; c<lex->length; c++){
        if ( lex->vars[c] != function) if (lex->vars[c]) link_free( lex->vars[c] );
    }
    
    function->refcount = 0;
    
    stack_free( lex->functions);
    free( lex->vars );
    free(lex);
}


static NativeType t;

Link codeblock_literal2(Link module, size_t offset){
    Link link = object_create(t);
    CodeBlock cb = link->value.codeblock;
    
    cb->parent = link_dup(module);
    cb->bytecode   = cb->parent->value.module->bytecode +offset;
    return link;
}

Link codeblock_literal(Link parent, bytecode_t bytecode){
    CodeBlock parent_cb = parent->value.codeblock;
    Link link = object_create(t);
    CodeBlock cb = link->value.codeblock;
    cb->parent = link_dup(parent_cb->parent);
    cb->bytecode   = bytecode;
    return link;
}

static void  create(Link self){
    CodeBlock cb = self->value.codeblock = malloc(sizeof( struct CodeBlock));
    cb->parent          = NULL;
    cb->lexicals        = NULL;
    cb->lexical_cycles  = 0;
    cb->bytecode        = NULL;
}

static void destroy(Link self){
    CodeBlock cb = self->value.codeblock;
    link_free(cb->parent);

    if (self->attribs) dictionary_clear(self->attribs);
    
    if (self){
        free(self->value.codeblock);
        object_destroy(self);          
    }    
}

static NATIVECALL(function_with){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    Link ret;
    
    if (argn <1) return exception("ArgumentError",NULL,NULL);
        
    if (argn ==2){
        if (args[1]->type != Global->array_type) return exception("ArgumentError",NULL,NULL);
        ret = object_call( This, args[0] , args[1]);
    }else{
        Link newarg = array_new(0);
        ret = object_call( This, args[0] , newarg);
        link_free(newarg);
    }
    
    return ret ? ret : object_create(Global->null_type);
}


static Link call(Link self, Link This, Link Arg){
    return interpret(self, This, Arg);
}

void * codeblock_alloc(){
    return object_create(t);
}


void at_link_free(Link self){
    if (self->value.codeblock->lexicals ) {
        lexicals_free( self );
    }
}





static NATIVECALL(function_foreach){
    
    Array self = This->value.array;
    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn <1) return exception("ArgumentError",NULL,NULL);
        
    if (! self) return object_create(Global->null_type);
    

    /* create the array that will be returned */
    Link rets_link = array_new(argn);
    Array rets = rets_link->value.array;    
    array_nullify( rets_link );
    
    Link newargs_link = array_new(0);
    
    Link link;
    int count =0;
    int c;
    for( c=0; c<argn; c++){
        /* call function and get return value */
        link = object_call(This, args[c] ,newargs_link);
        
        if (link){
            if ( is_critical(link) ){
                link_free(rets_link);
                return link;                
            }
            rets->links[count++] = link;
            continue;
        }
    }

    /* resize array down to number of non null elements */
    array_size(newargs_link, count);
    
    link_free(newargs_link);
    return rets_link;
};




NativeType function_init(){    
    t = newNative();
        t->create        = create;
        t->destroy       = destroy;
        t->link_free     = at_link_free;
        t->call          = call;
    
    addNativeCall(t, "with", function_with);    
    addNativeCall(t, "foreach", function_foreach);    
    return t;
}


