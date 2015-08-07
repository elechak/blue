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
#include "../threading.h"


EXPORT void init(INITFUNC_ARGS);

static NativeType mutex_type;
static NativeType thread_type;
static NativeType asyncfunc_type;


/* AsyncFunc type */

static void af_destroy(Link self){
    if (self->value.vptr) link_free(self->value.vptr);
    object_destroy(self);
}

struct Async_data{
    Link This;
    Link Arg;
    Link Self;
};

static void _run_async(struct Async_data * data){
    Link asyncfunc = data->Self;
    Link callable  = asyncfunc->value.vptr;
    
    object_call(callable,  data->This, data->Arg);
    if (data->This)link_free(data->This);
    if (data->Arg)link_free(data->Arg);
    if (data->Self)link_free(data->Self);
    free(data);
}

static Link af_call(Link self, Link This, Link Arg){
    // create the structure that will hold the data passed to the new thread
    struct Async_data * ad = malloc(sizeof(struct Async_data));

    ad->This = NULL;
    ad->Arg  = NULL;
    ad->Self = NULL;
    
    // populate the structure 
    if (This) ad->This = link_dup(This);
    if (Arg)  ad->Arg  = object_copy(Arg);
    if (self) ad->Self = link_dup(self);
        
    // start the new thread, and return the thread handle
    thread_t thread_handle = run_async(_run_async , ad);
    
    Link thread = object_create(thread_type);
    thread->value.vptr = malloc(sizeof(thread_t));
    *((thread_t *)(thread->value.vptr)) = thread_handle;
    return thread;    
}

static NATIVECALL(make_async){
    Link * args = array_getArray(Arg);
    Link self = object_create(asyncfunc_type);
    self->value.vptr = link_dup(args[0]);
    return self;
}


/* THREAD */

static void thread_destroy(Link self){
    free(self->value.vptr);
    object_destroy(self);
}

static NATIVECALL(thread_join){
    thread_t thread_handle = *((thread_t *)(This->value.vptr));
    join_async(thread_handle);
    return object_create(Global->null_type);
}




/* MUTEX */

static void mutex_destroy(Link self){
    mutex_free(self->value.vptr);
    object_destroy(self);
}

static NATIVECALL(mutex_new_native){
    Link link = object_create(mutex_type);
    link->value.vptr = mutex_new();
    return link;
}

static NATIVECALL(mutex_lock_native){
    mutex_lock(This->value.vptr);
    return object_create(Global->null_type);
}

static NATIVECALL(mutex_unlock_native){
    mutex_unlock(This->value.vptr);
    return object_create(Global->null_type);
}



/* INIT function */

void init(INITFUNC_ARGS){

    link_setThreadState(1);

    asyncfunc_type = newNative();
        asyncfunc_type->lib            = lib;
        asyncfunc_type->destroy        = af_destroy;
        asyncfunc_type->call           = af_call;
    
    
    thread_type = newNative();
        thread_type->lib                = lib;
        thread_type->destroy            = thread_destroy;
    
        addNativeCall(thread_type,"join", thread_join);
    
    mutex_type = newNative();
        mutex_type->lib                = lib;
        mutex_type->destroy            = mutex_destroy;

        addNativeCall(mutex_type,"lock", mutex_lock_native);
        addNativeCall(mutex_type,"unlock", mutex_unlock_native);

    //addCFunc(Module, "async", async);
    addCFunc(Module, "mutex", mutex_new_native);
    addCFunc(Module, "async", make_async);
}






