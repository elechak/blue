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


#ifndef _INDEX_
#define _INDEX_


#include "threading.h"
#include <stdlib.h>

struct Index{
    size_t capacity;
    size_t next;
    Mutex mutex;
    void ** entries;
};

typedef struct Index * Index;

#define index_get_fast(INDEX, I) (INDEX->entries[i])

Index index_new(size_t initial_size);
void index_free( Index self);
size_t index_add(Index self, void * data);
void * index_remove(Index self, size_t i);
void * index_get(Index self , size_t i);

#endif

