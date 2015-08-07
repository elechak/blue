/*
The blue programming language ("blue")
Copyright (C) 2007  Erik R Lechak

email: erik@leckak.info
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


#include "../global.h"
#include "../skiparray.h"

EXPORT void init(INITFUNC_ARGS);

static NativeType T = NULL;

typedef SkipArray TYPE;

static void destroyer(Link n){
    link_free(n);
}

static void  create(Link self){
    self->value.vptr =  NULL;
}

static void destroy(Link self){
    if (self->value.vptr) skiparray_free(self->value.vptr);
    self->value.vptr = NULL;
}


static Link getChild(Link self,Link what){
    TYPE t     = self->value.vptr;
    
    Link * args = array_getArray(what);
    size_t argn  = array_getLength(what);    
    
    if (! self->value.vptr) return NULL;
    if (argn != 1) return NULL;
    if (args[0]->type !=Global->number_type)return NULL;    
    
    long index = args[0]->value.number;    
    
    Link ret   = NULL;

    ret = skiparray_get(t, index);
    if (ret) return link_dup(ret);
        
    return NULL;
}

static Link delChild(Link self,Link what){
    TYPE t     = self->value.vptr;
    
    Link * args = array_getArray(what);
    size_t argn  = array_getLength(what);    
    
    if (! self->value.vptr) return NULL;
    if (argn != 1) return NULL;
    if (args[0]->type !=Global->number_type)return NULL;    
    
    long index =  args[0]->value.number;       
    
    Link ret = skiparray_delete(t, index);
    if (ret) return ret;

    return NULL;
}


static Link addChild(Link self,Link child, Link key){
    
    Link * args = array_getArray(key);
    size_t argn  = array_getLength(key);    
    
    if (argn != 1) return NULL;
    if (args[0]->type !=Global->number_type)return NULL;    
    
    long index =   args[0]->value.number;    
    

    if (! self->value.vptr){
        self->value.vptr = skiparray_new();
        skiparray_setDestroyer(self->value.vptr, (Destroyer)destroyer);
    }


    Link link = skiparray_replace(self->value.vptr, index , link_dup(child) );

    if (link){
        link_free(link);
    }else{
        skiparray_insert(self->value.vptr, index , child);
    }

    return child;
}



/* Native */
static NATIVECALL(list_new){
    return object_create(T);
}

static NATIVECALL(list_append){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (! This->value.vptr){
        This->value.vptr = skiparray_new();
        skiparray_setDestroyer(This->value.vptr, (Destroyer)destroyer);
    }

    size_t c;

    for (c=0 ; c < argn ; c++){
        skiparray_append(This->value.vptr, link_dup(args[c]) );
    }

    return link_dup(This);
};

static NATIVECALL(list_length){
    TYPE self = This->value.vptr;
    unsigned long length = self->length;
    return create_numberul(length);
}


void init(INITFUNC_ARGS){
    T = newNative();
        T->lib            = lib;
        T->create        = create;
        T->destroy        = destroy;
        T->addChild       = addChild;
        T->getChild       = getChild;
        T->delChild       = delChild;

    addNativeCall(T,"append", list_append);
    
    addCFunc(Module, "new", list_new);
    addNativeCall(T, "length", list_length);    
}







