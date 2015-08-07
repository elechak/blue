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

#ifndef _GLOBAL_
#define _GLOBAL_

#define  DPRINT(...)  fprintf(stderr,"%s:%d ",__FILE__, __LINE__);fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\n");fflush(stderr);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library_header.h"
#include "link.h"



struct Global{

    int threads;
    Link  backtrace_key;
    Link constructor_key;
    Link destructor_key;
    
    string_t  blue_location;
    string_t  blue_lib_location;

    NativeType critical_type;
    NativeType null_type;
    NativeType number_type;
    NativeType module_type;
    NativeType function_type;
    NativeType array_type;
    NativeType call_env_type;

    Link SYS_MODULE;
    
    unsigned long dbg_status;
};



EXPORT int blue_main(int argc, char **argv);

EXPORT extern struct  Global * Global;


EXPORT Link module_import(Link filename, Link args);
EXPORT Link create_null();

EXPORT Link create_string(const char * string);
EXPORT Link create_string_literal(void * string);
EXPORT Link create_string_str(string_t string);

EXPORT Link create_number(number_t x);
EXPORT Link create_numberd(double x);
EXPORT Link create_numberi(int x);
EXPORT Link create_numberul(unsigned long x);

EXPORT Link exception(char * type, string_t message, Link obj);

EXPORT extern Link (*array_new)(size_t);
EXPORT void array_size( Link self, int length);
EXPORT Link array_new_subarray( LinkStack source , size_t length);
EXPORT Link array_new_attach(LinkStack source);
EXPORT Link * array_getArray(Link self);
EXPORT size_t array_getLength(Link self);
EXPORT void array_set(Link self, int index, Link value);
EXPORT Link array_new_resize(Link link,  int size, Link def);


EXPORT void array_push(Link self, Link value);


EXPORT NATIVECALL(universal_print);


#endif


