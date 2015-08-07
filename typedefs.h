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
#ifndef _TYPEDEFS_
#define _TYPEDEFS_

#include <assert.h>
#include "stack.h"

#define AS(LINK, TYPE) (*((TYPE *)(LINK->data)))

#define alloc(TYPE , NAME) \
    TYPE NAME = malloc(sizeof(*NAME));\
    if (! NAME){\
        fprintf(stderr,"Memory Allocation Error %s:%d\n",__FILE__, __LINE__);\
        exit(EXIT_FAILURE);\
    }

typedef     unsigned char *              bytecode_t;
typedef     long                         offset_t;
typedef     double                       number_t;
typedef     struct Object *              Link;
typedef     enum ops                     operations_t;
typedef     struct NativeType *          NativeType;
typedef     struct Directory *           Workspace;

typedef Link  (*bin_math_func)     (Link, Link);

typedef Link (*NativeCall) (Link,Link);
 

#define INITFUNC_ARGS   void * lib, Link Module
typedef void    (*initfunc)          ( INITFUNC_ARGS );

/* Define the structure LinkStack see stack.h for details */
stack_define(LinkStack, Link);

typedef struct LinkArray{
    size_t size;
    Link links[];
}* LinkArray;



#endif

