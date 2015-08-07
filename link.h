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


#ifndef _LINK_
#define _LINK_

#include <stdlib.h>

#include "typedefs.h"
#include "library_header.h"

#include "bstring.h"
#include "native.h"
#include "interp.h"
#include "array.h"
#include "bcode.h"
#include "codeblock.h"
#include "dictionary.h"

struct Object{
    int refcount;           // the reference count for the object                 
    NativeType type;        // the type of the object -- its virtual function table
    struct Dictionary * attribs;      // this objects personal attributes
    union{                  
        unsigned long   ulong;
        string_t        string;
        number_t        number;
        struct Array *  array;
        Link            link;
        Module          module;
        CodeBlock       codeblock;
        NativeType      ntype;
        NativeCall      ncall;
        void *          vptr;
		int             i;
    }value;
};

void object_init();

/* dynamically enables thread support for object references */
EXPORT void link_setThreadState(int x);

EXPORT Link object_create(NativeType ntype);
EXPORT void object_destroy(Link self);

EXPORT extern void (*link_free)(Link self);
EXPORT extern Link (*link_dup)(Link self);

EXPORT inline number_t object_asNumber(Link self);
EXPORT inline string_t object_asString(Link self);
EXPORT inline string_t object_getString(Link self);

EXPORT inline Link object_getChild(Link parent, Link key);
EXPORT inline Link object_delChild(Link parent, Link key);
EXPORT inline Link object_addChild(Link parent, Link child, Link key);

EXPORT inline Link object_call(Link self, Link this_obj, Link args_obj);

EXPORT inline Link object_copy(Link self);
EXPORT int  object_is_true(Link self);
EXPORT inline int  object_compare(Link self, Link other);
EXPORT inline Link object_op_plus(Link self, Link other);
EXPORT inline Link object_op_minus(Link self, Link other);
EXPORT inline Link object_op_multiply(Link self, Link other);
EXPORT inline Link object_op_divide(Link self, Link other);
EXPORT inline Link object_op_modulus(Link self, Link other);
EXPORT inline Link object_op_power(Link self, Link other);
EXPORT inline Link object_op_neg(Link self);
EXPORT inline void object_draw(Link self);
 
EXPORT inline NativeType getNativeType(Link self);
 
 
#endif
