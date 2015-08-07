/*
The blue programming language ("blue")
Copyright (C) 2008  Erik R Lechak

email: erik@leckak.info
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

#include "../global.h"
#include <unistd.h>
#include <time.h>


EXPORT void init(INITFUNC_ARGS);


static NATIVECALL(random_int){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
	double i = 1.0;
	
    Link ret = NULL;
    
    switch (argn){
        case 0:
            ret = create_numberi( rand() );
            break;
        
        case 1:
            ret =create_numberi(  (int) (  object_asNumber(args[0])  * ( rand() / ( (double) RAND_MAX + i )   ))  );
            break;
        
        case 2:
            ret =create_numberi(  (int) (  (object_asNumber(args[1]) - object_asNumber(args[0]))  * ( rand() / ( (double) RAND_MAX + i)   ) + object_asNumber(args[0])  )  );
            break;
        
        default:
            ret = exception("NumberOfArgumentsError", NULL, NULL);
    }
    
    return ret;
};

static NATIVECALL(random_max){    
    return create_numberi(RAND_MAX);
};

static NATIVECALL(random_unit){    
    return create_numberd( rand() / ( (double) RAND_MAX )  );
};

void init(INITFUNC_ARGS){
    
    time_t seed;
    time(&seed);
    srand( (unsigned int ) seed );
    
    addCFunc(Module, "random", random_int);    
    addCFunc(Module, "unit", random_unit);    
    addCFunc(Module, "max", random_max);    
}


