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

#include "index.h"

#include <stdio.h>

/*
Index maps a void pointer to a size_t type index.
*/


Index index_new(size_t initial_size){
    Index self = malloc(sizeof(*self));
    self->entries = malloc(sizeof(void *) * initial_size);
    self->entries[0] = self;
    self->capacity = initial_size;
    self->next = 1;
    self->mutex = mutex_new();
    return self;
};


void index_free( Index self){
    if (self){
        if (self->entries) free(self->entries);
        if (self->mutex) mutex_free(self->mutex);
        free(self);
    }
}


size_t index_add(Index self, void * data){
    size_t c ; 
    
    mutex_lock(self->mutex);
    try_again:
    
    if (self->next < self->capacity){
        c = self->next++;
        self->entries[c] = data;
        mutex_unlock(self->mutex);
        return c;
    }
    
    // got here so we are maxed out

    // do a search for empty slots
    for (c = 1 ; c<self->capacity; c++){
        if (! self->entries[c]){
            self->entries[c] = data;
            mutex_unlock(self->mutex);
            return c;
        }
    }
    
    // got here so we are maxed out and no empty slots
    
    self->entries = realloc(self->entries , sizeof(void*) * self->capacity *2);
    self->next = self->capacity;
    self->capacity *=2;
    
    goto try_again;
    
}


void * index_remove(Index self, size_t i){
    void * entry = NULL;
    
    mutex_lock(self->mutex);
    
    if ( i < self->next){
        entry = self->entries[i]; // save the pointer
        self->entries[i] = NULL; // set the entry to NULL
        
        while(self->entries[self->next -1] == NULL ){
            self->next-- ;
        }
        
    }
    mutex_unlock(self->mutex);
    return entry;
}


void * index_get(Index self , size_t i){
    void * entry=NULL;
    mutex_lock(self->mutex);
    if ( i < self->next){
        entry = self->entries[i];        
    }
    mutex_unlock(self->mutex);
    return entry;
}

