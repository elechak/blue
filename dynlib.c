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
#include "dynlib.h"
#include <stdlib.h>
#include <stdio.h>

DynLibrary dynlib_load(char * library_name){
	DynLibrary lib = NULL;
#ifdef __WIN32__
	lib = LoadLibrary(library_name);
	//lib = LoadLibraryEx  might be needed for some of its extensions
	if (! lib){
		return NULL;
	}
#else
	//lib=dlopen(library_name, RTLD_NOW);
	lib=dlopen(library_name, RTLD_LAZY|RTLD_GLOBAL);
	if (! lib){
        fprintf(stderr, "%s\n",dlerror());
		return NULL;
	}

#endif
	return lib;
}



void * dynlib_getFunc(DynLibrary lib, char * funcname){
	void * func = NULL;
#ifdef __WIN32__
	func = GetProcAddress(lib,funcname);
#else
	func = dlsym(lib, funcname);
#endif
	return func;
}


void dynlib_free(DynLibrary lib){
#ifdef __WIN32__
	FreeLibrary(lib);
#else
    dlclose(lib);
#endif
	lib=NULL;
}



