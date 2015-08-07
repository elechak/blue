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

#ifndef __DYNLIB__
#define __DYNLIB__


#ifdef __WIN32__
#include <Windows.h>
typedef HINSTANCE DynLibrary;
#else
#include <dlfcn.h> // make sure to compile with -ldl
typedef  void *  DynLibrary;
#endif

DynLibrary dynlib_load(char * library_name);
void * dynlib_getFunc(DynLibrary lib, char * funcname);
void dynlib_free(DynLibrary lib);

#endif
