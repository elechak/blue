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

#include "../global.h"


EXPORT void init(INITFUNC_ARGS);

struct Dict{
    Dictionary dictionary;
    Mutex mutex;
};
typedef struct Dict * Dict;


static NativeType dict_type;

static void  create(Link self){
    self->value.vptr =  NULL;
}


static void destroy(Link self){
    Dict dict = self->value.vptr;
    if (dict){
        dictionary_free(dict->dictionary);
        mutex_free(dict->mutex);
        free(self->value.vptr);
    }
    object_destroy(self);   
}


static Link addChild(Link self,Link value,Link keys){
    
    Link * args = array_getArray(keys);
    size_t argn  = array_getLength(keys);       
    
    if (argn != 1) return NULL;

    string_t key = object_getString(args[0]);
    
    if ( ! key ) return NULL;
    
    Dict dict = self->value.vptr;
    if ( ! dict){
        dict = self->value.vptr = malloc( sizeof( *dict) );
        dict->dictionary = dictionary_new();
        dict->mutex = mutex_new();
    }
    
    mutex_lock(dict->mutex);    
    Link old = dictionary_insert(dict->dictionary, key, value );
    mutex_unlock(dict->mutex);
    if (old) link_free(old);
    
    return value; // return original not duplicate child
}

static Link getChild(Link self,Link keys){

    Dict dict = self->value.vptr;
    
    if ( ! dict) return NULL;
    if (! dict->dictionary) return NULL;
    
    Link * args = array_getArray(keys);
    size_t argn  = array_getLength(keys);       
    
    if (argn != 1) return NULL;
    
    
    mutex_lock(dict->mutex);    
    Link ret = dictionary_get(dict->dictionary, object_getString(args[0]));
    mutex_unlock(dict->mutex);    
    
    if ( ! ret) return NULL;
        
    return ret;
}

static Link delChild(Link self,Link keys){
    Dict dict = self->value.vptr;
    
    if ( ! dict ) return NULL;
    
    Link * args = array_getArray(keys);
    size_t argn  = array_getLength(keys);       
    
    if (argn != 1) return NULL;

    
    mutex_lock(dict->mutex);    
    Link ret = dictionary_delete(dict->dictionary, object_getString(args[0]));
    mutex_unlock(dict->mutex);        
    
    if ( ! ret) return NULL;
        
    return ret;
}


static NATIVECALL(dict_create){
    return object_create(dict_type);
};

static NATIVECALL(dict_length){
    Dict dict = This->value.vptr;
    if (! dict) return create_numberul(0);
    size_t size =  dict->dictionary->size;
    return create_numberul(size);
}

static NATIVECALL(dict_keys){
    Dict dict = This->value.vptr;
    mutex_lock(dict->mutex);    
    Link ret = dictionary_getKeys(dict->dictionary);
    mutex_unlock(dict->mutex);    
    return ret;
}

static NATIVECALL(dict_values){
    Dict dict = This->value.vptr;
    mutex_lock(dict->mutex);    
    Link ret = dictionary_getValues(dict->dictionary);
    mutex_unlock(dict->mutex);    
    return ret;
}





void init(INITFUNC_ARGS){

    dict_type = newNative();
        dict_type->create         = create;
        dict_type->destroy        = destroy;
        dict_type->addChild       = addChild;
        dict_type->getChild       = getChild;
        dict_type->delChild       = delChild;
    
    addCFunc(Module,"new", dict_create);
    addNativeCall(dict_type, "length", dict_length);
    addNativeCall(dict_type, "keys", dict_keys);
    addNativeCall(dict_type, "values", dict_values);
}





