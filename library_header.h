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


#ifndef __LIB_HEADER__
#define __LIB_HEADER__

#define EXPORT


    //~ #ifdef __WIN32__
    //~ // WINDOWS

        //~ #ifdef BUILD_DLL
        //~ /* DLL export */
        //~ #define EXPORT __declspec(dllexport)
        //~ #else
        //~ /* EXE import */
        //~ #define EXPORT __declspec(dllimport)
        //~ #endif

    //~ #else
    //~ // LINUX
    
        //~ #ifdef BUILD_DLL
    
            //~ #define EXPORT __attribute__ ((visibility("default")))
            //~ //#pragma GCC visibility push(hidden)
            
        //~ #else

            //~ #define EXPORT
            
        //~ #endif

    //~ #endif


#endif

