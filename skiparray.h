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

#ifndef _SPLAYTREE_
#define _SPLAYTREE_

#include "comparison.h"
#include <stdlib.h>

typedef struct SkipArrayBlock * SkipArrayBlock;
typedef struct SkipArray * SkipArray;

struct SkipArray{
    size_t length;
    size_t blocksize;
    SkipArrayBlock root;
    Destroyer destroyer;
};

SkipArray skiparray_new();
void skiparray_free(SkipArray self);

void skiparray_setDestroyer(SkipArray self , Destroyer d);
void * skiparray_get(SkipArray self, size_t index);
void skiparray_insert(SkipArray self, size_t index, void * data);
void * skiparray_delete(SkipArray self, size_t index);
void skiparray_append(SkipArray self, void * data);
size_t skiparray_find(SkipArray self, void * key , Comparator comp);
void * skiparray_replace(SkipArray self, size_t index, void * data);

#endif
