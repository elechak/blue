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

#ifndef _ARRAY_
#define _ARRAY_

#include "global.h"

struct Array{
    size_t length;
    Link  links[];
};
typedef struct Array * Array;

LinkArray linkarray_new(size_t size);
void linkarray_free( LinkArray la);

Link array_new_nothreads(size_t);
Link array_new_threads(size_t);

void array_destroy_nothreads(Link);
void array_destroy_threads(Link);

NativeType array_init();
void array_nullify(Link self);

#endif



