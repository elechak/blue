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

#include "global.h"
#include "threading.h"
#include "dynlib.h"
#include "number.h"

static NativeType type_type       = NULL;
static NativeType cfunction_type  = NULL;
static NativeType critical_type   = NULL;

static Mutex mutex_attribs[16];

static inline Mutex getAttribsMutex(Link self){
    return  mutex_attribs[ ((((unsigned long)self) & 0xF0)>>4) ];
}

/* Default functions for virtual function table */

static Link def_copy(Link self){
    return NULL;
}

static number_t def_asNumber(Link self){
    return (number_t) 0;
}

static string_t def_asString(Link self){
    return string_new_formatted("object%p", self);
}


static int def_is_true(Link self){
    return 0;
}

static Link def_call(Link self, Link This, Link Arg){
    return exception("NotCallable", NULL, NULL);
}


NATIVECALL(universal_print){    
    size_t count;
    string_t s = NULL;
        
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    s = object_asString(This);
    fwrite( s->data, 1, s->length, stdout);
    free(s);
    
    for ( count = 0; count <argn; count++){
        s = object_asString(args[count]);
        fwrite(s->data, 1,s->length,stdout);
        free(s);
    }
    fflush(stdout);
    return create_null();
};


Link addAttr(Link self,Link value,Link key){
    Mutex mutex = getAttribsMutex(self);
    mutex_lock(mutex);
    if (! self->attribs) self->attribs = dictionary_new();
    Link old = dictionary_insert(self->attribs, object_getString(key), value); 
    mutex_unlock(mutex);
    
    if (old) link_free(old);
    return value; // return original not duplicate child
}


Link getAttr(Link self,Link key){

    string_t key_string = object_getString(key);
    
    /* attribute keys are always strings */
    if (! key_string ) return NULL;
    
    Link ret = NULL;
    Mutex mutex = NULL;

    /* look for attribute in the object */
    if (self->attribs){    
        mutex = getAttribsMutex(self);
        mutex_lock(mutex);
        ret = dictionary_get(self->attribs, key_string); 
        mutex_unlock(mutex);
    }

    /* look for attribute in the type */
    if (! ret){
        if (self->type->type_link->attribs){    
            mutex = getAttribsMutex(self->type->type_link);
            mutex_lock(mutex);
            ret = dictionary_get(self->type->type_link->attribs, key_string); 
            mutex_unlock(mutex);
        }
    }
    
    return ret;
}

Link delAttr(Link self,Link key){
    if (! self->attribs) return NULL;
    Link obj = NULL;
    Mutex mutex = getAttribsMutex(self);
    mutex_lock(mutex);
    obj  =   dictionary_delete(self->attribs, object_getString(key));
    mutex_unlock(mutex);
    return obj;
}


Link getAttrKeys(Link self){
    Mutex mutex = getAttribsMutex(self);
    mutex_lock(mutex);
    Link ret = dictionary_getKeys(self->attribs);
    mutex_unlock(mutex);    
    return ret;
}


static int def_comp( Link a, Link b){
    return b-a;
}


static Link ret_null(){
    return NULL;
}

static string_t ret_null_str(){
    return NULL;
}


NativeType newNative(){
    
    alloc( NativeType, self);

    self->lib = NULL;
    self->type_link = NULL;
    
    // Vtable initializ ation
    self->create        = NULL;
    self->destroy       = object_destroy;
    self->link_free     = NULL;
    self->copy          = def_copy;
    self->getChild      = ret_null;
    self->delChild      = ret_null;
    self->addChild      = ret_null;
    self->call          = def_call;
    self->asNumber      = def_asNumber;
    self->asString      = def_asString;
    self->getString     = ret_null_str;
    self->is_true       = def_is_true;
    self->compare       = def_comp;
    self->op_plus       = ret_null;
    self->op_minus      = ret_null;
    self->op_multiply   = ret_null;
    self->op_divide     = ret_null;
    self->op_modulus    = ret_null;
    self->op_power      = ret_null;
    self->op_neg        = ret_null;
    self->draw          = NULL;

    if (type_type){
            self->type_link                  = object_create(type_type);
            self->type_link->value.ntype     = self;
    }
    return self;
}

NativeType extendNative(NativeType base){
    alloc( NativeType, self);

    if (base){
        memcpy(self, base, sizeof(*self));
    }else{
        return newNative();
    }

    self->lib = NULL;

    return self;
}


int loadNative(char * filename, Link Module){
    void *lib = NULL;
    initfunc typeInit;

    lib = dynlib_load(filename);

    if (! lib) return -1;

    typeInit = dynlib_getFunc(lib,"init");
    if (typeInit){
        typeInit(lib, Module);  // INITFUNC_ARGS
    }else{
        dynlib_free(lib);
        return -2;
    }
    return 0;
}


void addCFunc(Link self, char * name, NATIVECALL( (*func))){
    Link key = create_string(name);
    Link value = object_create(cfunction_type);
    value->value.ncall = func;
    addAttr(self,value,key);
    link_free(value);
    link_free(key);
}

void addNativeCall(NativeType ntype, char *funcname, NATIVECALL((*func))){
    Link key = create_string(funcname);
    Link value = object_create(cfunction_type);
    value->value.ncall = func;
    addAttr(ntype->type_link, value, key);
    link_free( key);
}


/* TYPE-TYPE */

/* calling a type object will create an object of this type */
static Link type_type_call(Link self,  Link This, Link Arg){
    NativeType ntype = self->value.ntype;
    
    link_dup( ntype->type_link );
    Link obj = object_create( ntype );
    
    Link constructor = getAttr(obj, Global->constructor_key);
    
    if (constructor){
            object_call(constructor, obj, Arg);
            link_free(constructor);
    }

    return obj;
}

static void type_type_destroy(Link self){
    NativeType nt = self->value.ntype;
    nt->type_link = NULL;
    free(nt);
    object_destroy(self);  
}

static string_t type_type_asString(Link self){
    return string_new_formatted("type[%p]", self->value.ntype);
}



static void destroy_with_destructor(Link self){
    Link destructor = getAttr(self, Global->destructor_key);
    if (destructor){
        link_dup(self); // do this so the object is not destroyed before the destructor call
        object_call(destructor, self, NULL);
        link_free(destructor);
    }
    link_free(self->type->type_link);
    object_destroy(self);
}

static void _extend(string_t key, Link value, void *  target){
    if (((Link)target)->attribs) dictionary_insert( ((Link)target)->attribs, key, value);
}

static void extend(Link self, Link parent){
    if (! self->attribs) self->attribs = dictionary_new();
    dictionary_each(parent->attribs, _extend, self);
}

static void _contract(string_t key, Link value,  void * target){
    link_free( dictionary_delete( ((Link)target)->attribs,key) );
}

static void contract(Link self, Link parent){
    if (self->attribs) dictionary_each(parent->attribs, _contract, self);
}




static NATIVECALL(extendType){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);    
        
    int c;
    for (c=0; c<argn ; c++){
        extend(This, args[c]);
    }
    
    return link_dup(This);    
}

static NATIVECALL(newType){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);    
    
    NativeType nt = newNative();
    nt->destroy = destroy_with_destructor;
    
    int c;
    for (c=0; c<argn ; c++){
        extend(nt->type_link, args[c]);
    }
    
    return link_dup(nt->type_link);
};


/* C-FUNCTIONS */
typedef Link (*cfunc)(Link);

static string_t cfunction_asString(Link self){
    return string_new_formatted("<cfunction %p>",self->value.ncall);
}

static Link cfunction_call(Link self, Link This, Link Arg){
    return (self->value.ncall)(This, Arg) ;
}


/* CRITICAL */
Link create_critical(Link raised){
    assert( raised );
    Link crit = object_create(critical_type);
    crit->value.link = raised;
    return crit;
}

static void critical_destroy(Link self){
    if ( self->value.link ) link_free( self->value.link);
    if ( self->attribs ) dictionary_free(self->attribs);
    self->attribs = NULL;
    object_destroy(self);
}

static string_t critical_asString(Link self){
    return object_asString(self->value.link);
}

static string_t exc_sep;

Link exception(char * type, string_t message, Link obj){
    string_t s = string_new( type );
    if ( message ){
        s = string_new_merge( s, exc_sep, 0,1);
        s = string_new_merge( s, message, 0,1);
    }
    return create_critical( create_string_str(s) );
}



// NULL
Link create_null(){ 
    return object_create(Global->null_type);
}

static Link null_copy(Link self){
    return create_null();
}

static string_t null_asString(Link self){
    return string_new("()");
}


// User Type
static NATIVECALL(utype_extend){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);    
    
    int c;
    if (argn){
        for (c=1; c<argn ; c++){
            extend(args[0], args[c]);
        }
        return link_dup(args[0]);
    }
    
    return create_null();    
}

static NATIVECALL(utype_contract){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);    
    
    int c;
    if (argn){
        for (c=1; c<argn ; c++){
            contract(args[0], args[c]);
        }
        return link_dup(args[0]);
    }
    
    return create_null();    
}



static Link utype_copy(Link self){
    NativeType nt = newNative();
    
    nt->destroy = destroy_with_destructor;
    nt->copy    = utype_copy;
        
    return link_dup(nt->type_link);        
}


static NATIVECALL(utype_new){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);    
    
    NativeType nt = newNative();
    nt->destroy = destroy_with_destructor;
    nt->copy    = utype_copy;
    
    int c;
    for (c=0; c<argn ; c++){
        extend(nt->type_link, args[c]);
    }
    
    return nt->type_link;
};


static NATIVECALL(sys_clone){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);  

    if (argn != 1) return exception("CloneFailure",NULL,NULL);
    Link link = args[0]->type->copy(args[0]);
    if (! link) return exception("CloneFailure",NULL,NULL);
    
    extend(link, args[0]);
    return link;
}


static NATIVECALL(sys_copy){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);  

    if (argn != 1) return exception("CopyFailure",NULL,NULL);
    Link link = args[0]->type->copy(args[0]);
    if (! link) return exception("CopyFailure",NULL,NULL);
    
    return link;
}


void native_init(){   
    exc_sep = string_new(": ");

    int c;
    for (c =0; c < 16; c++){
        mutex_attribs[c] = mutex_new();
    }    
   
    type_type = newNative();
        type_type->destroy                 = type_type_destroy;
        type_type->asString                = type_type_asString;
        type_type->call                    = type_type_call;
        type_type->type_link               = object_create(type_type);
        type_type->type_link->value.ntype  = type_type;

    cfunction_type = newNative();
        cfunction_type->asString       = cfunction_asString;
        cfunction_type->call           = cfunction_call;
    
    str_init(); 
    
    addNativeCall(type_type, "new", newType);
    addNativeCall(type_type, "extend", extendType);
    
    Global->critical_type = critical_type = newNative();
        critical_type->destroy    = critical_destroy;
        critical_type->asString   = critical_asString;
    
    Global->null_type                = newNative(); 
        Global->null_type->asString  = null_asString;
        Global->null_type->copy      = null_copy;
        addNativeCall(Global->null_type, "print", universal_print);
        
        
    Global->module_type               = module_init();    
    Global->number_type               = number_init();
    Global->function_type             = function_init();
    Global->array_type                = array_init();        
          
    extend(cfunction_type->type_link, Global->function_type->type_link);


    addCFunc(Global->SYS_MODULE,"class", utype_new);    
    addCFunc(Global->SYS_MODULE,"extend", utype_extend);            
    addCFunc(Global->SYS_MODULE,"contract", utype_contract);            
    addCFunc(Global->SYS_MODULE,"clone", sys_clone);            
    addCFunc(Global->SYS_MODULE,"copy", sys_copy);            
        
}


