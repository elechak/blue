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

#include "link.h"
#include "threading.h"
#include "number.h"


static Mutex mutex_refcount[16];

static inline Mutex getRefcountMutex(Link self){
    return  mutex_refcount[ ((((unsigned long)self) & 0xF0)>>4) ];
}

void object_init(){    
    int c;
    for (c =0; c < 16; c++){
        mutex_refcount[c] = mutex_new();
    }
}


/* Create an object of the requested type, return the initial Link */
Link object_create(NativeType ntype){
    Link self = malloc(sizeof( *self));

    // initialize this object
    self->refcount  = 1;
    self->type      = ntype;
    self->attribs = NULL;

    if (self->type->create) self->type->create(self);        
    return self;
}

/* Free the object's allocated memory, it does not free any memory allocated by the obejct (see vtable's destroy function) */
void object_destroy(Link self){
        if (self->attribs) dictionary_free(self->attribs);
        free(self);    
}



/* LINK_FREE */
/* decrements the refcount and initiates garbage collection if required */

static void _link_free_threads(Link self){
    if (self->type->link_free) self->type->link_free(self);
    Mutex mutex = getRefcountMutex(self);
    mutex_lock(mutex);
    int rc = --(self->refcount);
    mutex_unlock(mutex);  
    if (self->type->link_free) self->type->link_free(self);
    if (! rc) self->type->destroy(self); 
}

static void _link_free_nothreads(Link self){
    self->refcount--;
    if (self->type->link_free) self->type->link_free(self);    
    if ( self->refcount == 0) self->type->destroy(self);     
}

void (*link_free)(Link self) = _link_free_nothreads;


/* LINK_DUP */
/* increments the refernce count for specified object */

static Link _link_dup_nothreads(Link self){
    if (! self) return NULL;
    self->refcount++;
    return self;
}

static Link _link_dup_threads(Link self){
    if (! self) return NULL;
    
    Mutex mutex = getRefcountMutex(self);
    mutex_lock(mutex);
    self->refcount++;
    mutex_unlock(mutex);    
    return self;
}

Link (*link_dup)(Link self) = _link_dup_nothreads;


/* Main function used to enable or disable threading support */
void link_setThreadState(int x){
    switch(x){
        case 0:
            link_free = _link_free_nothreads;
            link_dup = _link_dup_nothreads;
            Global->number_type->destroy = number_destroy_nothreads;
            number_new = number_new_nothreads;
            Global->array_type->destroy = array_destroy_nothreads;
            array_new = array_new_nothreads;
            threads_disable();
            Global->threads = 0;
            break;        
        case 1:
            link_free = _link_free_threads;
            link_dup = _link_dup_threads;
            Global->number_type->destroy = number_destroy_threads;
            number_new = number_new_threads;        
            Global->array_type->destroy = array_destroy_threads;
            array_new = array_new_threads;        
            Global->threads = 1;
            threads_enable();
            break;
    }
}



/* VIRTUAL FUNCTION TABLE CALLS */

inline number_t object_asNumber(Link self){
    return self->type->asNumber(self);
}

inline string_t object_asString(Link self){
    return self->type->asString(self);
}

inline string_t object_getString(Link self){
    return self->type->getString(self);
}

/* CHILDREN */
inline Link object_getChild(Link self, Link key){
    return self->type->getChild(self,key);
}

inline Link object_delChild(Link self, Link key){
    return self->type->delChild(self,key);
}

inline Link object_addChild(Link self, Link child, Link key){
    return self->type->addChild(self,child,key);
}



/* CALLS */
inline Link object_call(Link self, Link this_obj, Link args_obj){
    return self->type->call(self, this_obj, args_obj);
}


inline Link object_copy(Link self){
    return self->type->copy(self);
}

inline int object_is_true(Link self){
    return self->type->is_true(self);
}

inline int object_compare(Link self, Link other){
    return self->type->compare(self,other);
}

inline Link object_op_plus(Link self, Link other){
    return self->type->op_plus(self,other);
}

inline Link object_op_minus(Link self, Link other){
    return self->type->op_minus(self,other);
}

inline Link object_op_multiply(Link self, Link other){
    return self->type->op_multiply(self,other);
}

inline Link object_op_divide(Link self, Link other){
    return self->type->op_divide(self,other);
}

inline Link object_op_modulus(Link self, Link other){
    return self->type->op_modulus(self,other);
}

inline Link object_op_power(Link self, Link other){
    return self->type->op_power(self,other);
}

inline Link object_op_neg(Link self){
    return self->type->op_neg(self);
}

inline void object_draw(Link self){
    return self->type->draw(self);
}

inline NativeType getNativeType(Link self){
    return self->type;
}











