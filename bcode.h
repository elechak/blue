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

#ifndef _BCODE_
#define _BCODE_

#include "global.h"
#include "compiler.h"
#include "assembler.h"



struct Module{
    string_t name;
    bytecode_t bytecode;
    size_t bytecode_size;
    LinkArray global_vars;
};
typedef struct Module * Module;


Link module_new(char * filename, Link Args);
void module_save( Link module, char * filename );


Link create_module_string(const string_t source);
Link create_module_filename(const char * filename);

NativeType module_init();
#endif
