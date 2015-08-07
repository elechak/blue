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

#include "../../global.h"
#include "md5.h"


EXPORT void init(INITFUNC_ARGS);


static NATIVECALL(hash_md5){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    string_t s;
    
    if ((argn !=1) || (! (s = object_getString(args[0])))) return exception("HashArgError", NULL,NULL);
    
    struct cvs_MD5Context context;
	unsigned char checksum[16];
    

    cvs_MD5Init (&context);
    cvs_MD5Update (&context, (unsigned char *)s->data, s->length);
    cvs_MD5Final (checksum, &context);
    
    s = string_new_ls( 32, NULL);
    
    char * dest = s->data;

    sprintf( dest , "%02x%02x%02x%02x", checksum[0],checksum[1],checksum[2],checksum[3]  );
    sprintf( dest+8 , "%02x%02x%02x%02x", checksum[4],checksum[5],checksum[6],checksum[7]  );
    sprintf( dest+16 , "%02x%02x%02x%02x", checksum[8],checksum[9],checksum[10],checksum[11]  );
    sprintf( dest+24 , "%02x%02x%02x%02x", checksum[12],checksum[13],checksum[14],checksum[15]  );
	

    return create_string_str( s );
}



void init(INITFUNC_ARGS){
    addCFunc(Module, "md5", hash_md5);
}



