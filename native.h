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
#ifndef _NATIVE_
#define _NATIVE_

#include "library_header.h"
#include "bstring.h"
#include "typedefs.h"


#define NATIVECALL(NAME) Link NAME(Link This, Link Arg)


struct NativeType{
    void      *lib;

    Link     type_link;

    void     (*create)       (Link);
    void     (*destroy)      (Link);

    void     (*link_free)    (Link);     
    
    Link     (*copy)         (Link);

    Link     (*call)         (Link ,Link, Link ); // function, this, args

    number_t (*asNumber)     (Link);
    string_t (*asString)     (Link);
    string_t (*getString)    (Link);

    Link     (*getChild)     (Link, Link);
    Link     (*delChild)     (Link, Link);
    Link     (*addChild)     (Link, Link, Link);

    int      (*is_true)      (Link);
    int      (*compare)      (Link,Link);

    Link     (*op_plus)      (Link , Link );
    Link     (*op_minus)     (Link , Link );
    Link     (*op_multiply)  (Link , Link );
    Link     (*op_divide)    (Link , Link );
    Link     (*op_modulus)   (Link , Link );
    Link     (*op_power)     (Link , Link );
    Link     (*op_neg)       (Link );

    void     (*draw)         (Link);
};

void native_init();

EXPORT void type_addType(NativeType nt, char * name);

EXPORT NativeType newNative();
EXPORT NativeType extendNative(NativeType base);
EXPORT int loadNative(char * filename, Link Module);


EXPORT void addNativeCall(NativeType ntype, char *funcname, NATIVECALL( (*func)));
EXPORT void addCFunc(Link self, char * name, NATIVECALL( (*func))); // add native method to object self

EXPORT Link addAttr(Link self,Link value,Link key);
EXPORT Link getAttr(Link self,Link key);
EXPORT Link delAttr(Link self,Link key);
EXPORT Link getAttrKeys(Link self);

EXPORT Link create_critical(Link raised);
#define is_critical( LINK ) (LINK->type==Global->critical_type)


#endif
