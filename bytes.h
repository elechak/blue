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
#ifndef _BYTES_
#define _BYTES_

#include <stdlib.h>


struct Bytes{
    size_t         capacity;
    size_t         write_offset;
    size_t         read_offset;
    unsigned char  *data;
    size_t         grow_size;
};
typedef struct Bytes *Bytes;

Bytes bytes_new();
Bytes bytes_attach(unsigned char * data, size_t size);
void * bytes_detach(Bytes self);
void bytes_free(Bytes self);
void bytes_clear(Bytes self);
void bytes_rseek(Bytes self, size_t offset);

size_t bytes_length(Bytes self);
void * bytes_atOffset(Bytes self, size_t offset);
void * bytes_asPointer(Bytes self);

/* Unformatted write and read */
void bytes_write(Bytes self, const void *ptr, size_t size);
void bytes_read(Bytes self, void *ptr, size_t size);

void bytes_writeRewind(Bytes self,size_t size);

/* Formatted write and read */
void bytes_writef(Bytes self, const char *format, ...);
void bytes_readf(Bytes self, const char *format, ...);
/* File operations */
void bytes_save(Bytes self, const char * filename);
Bytes bytes_load(const char * filename);
/* show the state  */
void bytes_debug(Bytes bs);

/* Array functions */
/*
* write to the memory location
* returns where the next element should be written
* example:   bwrite(array, &the_int, sizeof(int), 1)
*/
void * bwrite(void *memory, const void *ptr, size_t size, size_t nmemb);
/*
* read from memory location and sets ptr
* example:    bread(array, &the_int, sizeof(int), 1)
*/
void * bread(const void *memory, void *ptr, size_t size, size_t nmemb);


//~ int main(int argc, char **argv){
    //~ int x, y, z;
    //~ Bytes bs = bytes_new();
    //~ bytes_writef(bs, "iii",5,10,15);
    //~ bytes_debug(bs);
    //~ bytes_readf(bs, "iii", &x,&y,&z);
    //~ printf(" %i %i %i \n", x,y,z);
    //~ return 0;
//~ }

#endif
