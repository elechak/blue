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

#ifndef _INTERP_
#define _INTERP_


#include "global.h"

#include "stack.h"
#include "assembler.h"
#include "array.h"
#include "str.h"


NativeType call_env_init();
EXPORT Link interpret(Link , Link , Link );

struct CallEnv{
    struct Module * module;  // current module containing current function
    Link function;           // current executing function
    bytecode_t start;        // start of the modules bytecode
    bytecode_t current;      // current position in bytecode
    Link Self;               // value of self
    Link This;               // value of this
    Link Arg;                // arguments
    Link Def;                // default object
    LinkArray local;         // local variables (lookup via int index)
    int linenum;             // current line number if debug info is enabled ( -g flag)
    LinkStack stack;         // main stack for all expression execution
    int stack_offset;        // offset denoting where this environments section of the stack begins
    struct CallEnv * next;   // linked storage for callenv reuse
    struct CallEnv * parent; // parent of this call environment, root callenv will have a null parent
    int lexical_root;
};
typedef struct CallEnv * CallEnv;


#endif








