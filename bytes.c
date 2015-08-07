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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "bytes.h"

static void bytes_alloc(Bytes self, size_t size);
static void bytes_grow(Bytes self, size_t size);

void bytes_debug(Bytes bs){
    fprintf(stderr,"\nBS capacity=%lu  wo=%lu  ro=%lu\n", bs->capacity, bs->write_offset, bs->read_offset);
    int c;
    for(c=0; c<bs->write_offset; c++){
        fprintf(stderr,"%i ", bs->data[c]);
    }
    fprintf(stderr,"\n");
}

Bytes bytes_new(){
    Bytes self = malloc(sizeof(struct Bytes));
    if (! self){
        printf("Memory allocation Error\n");
        exit(1);
    }
    self->data           = NULL;
    self->capacity       = 0;
    self->write_offset   = 0;
    self->read_offset    = 0;
    self->grow_size      = 128;
    return self;
}


Bytes bytes_attach(unsigned char * data, size_t size){
    Bytes bytes = bytes_new();
    bytes->data = data;
    bytes->capacity = size;
    bytes->write_offset = size;
    return bytes;
}

void * bytes_detach(Bytes self){
    void * ret = self->data;
    self->data=NULL;
    bytes_free(self);
    return ret;
}



void bytes_free(Bytes self){
    if (self->data){
        free(self->data);
    }
    free(self);
}

static void bytes_alloc(Bytes self, size_t size){
    self->data = realloc(self->data, size);
    self->capacity = size;

    if (! self->data){
        printf("Memory allocation Error\n");
        exit(1);
    }
}

static void bytes_grow(Bytes self, size_t size){
    if (size >= self->grow_size){
        bytes_alloc(self, size + self->capacity);
    }else{
        bytes_alloc(self, self->grow_size + self->capacity);
    }
}

void bytes_clear(Bytes self){
    self->write_offset = 0;
    self->read_offset = 0;
}

void bytes_rseek(Bytes self, size_t offset){
    self->read_offset = offset;
}


void * bytes_asPointer(Bytes self){
    return self->data;
}

size_t bytes_length(Bytes self){
    return self->write_offset;
}

void * bytes_atOffset(Bytes self, size_t offset){
    return self->data + offset;
}


void bytes_writeRewind(Bytes self,size_t size){
    self->write_offset -=size;
}

void bytes_write(Bytes self, const void *ptr, size_t size){

    /* ensure that the bytes is large enough to hold the incomming data */
    if (self->write_offset + size >= self->capacity)
        bytes_grow(self,size);

    /* now copy the memory to the bytes */
    memcpy(self->data+self->write_offset, ptr, size);
    self->write_offset += size;
}


void bytes_read(Bytes self, void *ptr, size_t size){
    memcpy(ptr, self->data+self->read_offset,  size);
    self->read_offset += size;
}


void bytes_writef(Bytes self, const char *format, ...){
    va_list va;
    va_start(va, format);
    size_t count;
    size_t size;

    float f;
    double d;
    int i;
    unsigned int I;
    long int l;
    unsigned long int L;
    char c;
    unsigned char C;
    char * s      = NULL;
    size_t z;


    while(*format){
        while(*format && *format==' ') format++;  // skip whitespace
        if (! *format) break;

        // get count if it exists
        if (*format =='*'){
            count=va_arg(va, size_t);
            format++;
        }else{
            count = strtol(format,(char **)&format,10);
            if ( count == 0 ) count=1;
        }
        
        switch(*format){            
            case 'f': // FLOAT
                for( ; count > 0 ; --count){
                    f=va_arg(va, double);
                    bytes_write(self, &f, sizeof(float));
                }
                break;
            case 'd': // DOUBLE
                for( ; count > 0 ; --count){
                    d=va_arg(va, double);
                    bytes_write(self, &d, sizeof(double));
                }
                break;
            case 'i': // INT
                for( ; count > 0 ; --count){
                    i=va_arg(va, int);
                    bytes_write(self, &i, sizeof(int));
                }
                break;
            case 'I': // UNSIGNED INT
                for( ; count > 0 ; --count){
                    I=va_arg(va, unsigned int);
                    bytes_write(self, &i, sizeof(unsigned int));
                }
                break;
            case 'l': // LONG
                for( ; count > 0 ; --count){
                    l=va_arg(va, long);
                    bytes_write(self, &l, sizeof(long));
                }
                break;
            case 'L': // UNSIGNED LONG INT
                for( ; count > 0 ; --count){
                    L=va_arg(va, unsigned long int);
                    bytes_write(self, &L, sizeof(unsigned long int));
                }
                break;
            case 'c': // CHAR
                for( ; count > 0 ; --count){
                    c=va_arg(va, int);
                    bytes_write(self, &c, sizeof(char));
                }
                break;
            case 'C': // UNSIGNED CHAR
                for( ; count > 0 ; --count){
                    C=va_arg(va, unsigned int);
                    bytes_write(self, &C, sizeof(unsigned char));
                }
                break;
            case 'z': // SIZE_T
                for( ; count > 0 ; --count){
                    z=va_arg(va, size_t);
                    bytes_write(self, &z, sizeof(size_t));
                }
                break;
            case 'b': // BYTE STRING
                s=va_arg(va, char *);
                bytes_write(self, s, count);
                break;                
            case 's': // NULL TERMINATED STRING
                for( ; count > 0 ; --count){
                    s=va_arg(va, char *);
                    size = strlen(s)+1;
                    bytes_write(self, s, size);
                }
                break;
            case 'S': // NULL TERMINATED STRING WITH OUT NULL
                for( ; count > 0 ; --count){
                    s=va_arg(va, char *);
                    size = strlen(s);
                    bytes_write(self, s, size);
                }
                break;
            case '.':
                c=0; // CHARACTER
                for( ; count > 0 ; --count){
                    bytes_write(self, &c,1 );
                }
                break;
        }
        format++;
    }
    va_end(va);
}




void bytes_readf(Bytes self, const char *format, ...){
    va_list va;
    va_start(va, format);
    size_t count;
    size_t size;

    float *f              = NULL;
    double *d             = NULL;
    int *i                = NULL;
    unsigned int *I       = NULL;
    long int *l           = NULL;
    unsigned long int *L  = NULL;
    char *c               = NULL;
    unsigned char *C      = NULL;
    char * s              = NULL;
    size_t *z             = NULL;


    while(*format){
        while(*format && *format==' ') format++;  // skip whitespace
        if (! *format) break;


        // get count if it exists
        if (*format =='*'){
            count=va_arg(va, size_t);
            format++;
        }else{
            count = strtol(format,(char **)&format,10);
            if ( count == 0 ) count=1;
        }

        switch(*format){
            case 'f':
                for( ; count > 0 ; --count){
                    f=va_arg(va, float *);
                    bytes_read(self, f, sizeof(float));
                }
                break;
            case 'd':
                for( ; count > 0 ; --count){
                    d=va_arg(va, double *);
                    bytes_read(self, d, sizeof(double));
                }
                break;
            case 'i':
                for( ; count > 0 ; --count){
                    i=va_arg(va, int *);
                    bytes_read(self, i, sizeof(int));
                }
                break;
            case 'I':
                for( ; count > 0 ; --count){
                    I=va_arg(va, unsigned int *);
                    bytes_read(self, i, sizeof(unsigned int));
                }
                break;
            case 'l':
                for( ; count > 0 ; --count){
                    l=va_arg(va, long int *);
                    bytes_read(self, l, sizeof(long int));
                }
                break;
            case 'L':
                for( ; count > 0 ; --count){
                    L=va_arg(va, unsigned long int *);
                    bytes_read(self, L, sizeof(unsigned long int));
                }
                break;
            case 'c':
                for( ; count > 0 ; --count){
                    c=va_arg(va, char *);
                    bytes_read(self, c, sizeof(char));
                }
                break;
            case 'C':
                for( ; count > 0 ; --count){
                    C=va_arg(va, unsigned char *);
                    bytes_read(self, C, sizeof(unsigned char));
                }
                break;
            case 'z':
                for( ; count > 0 ; --count){
                    z=va_arg(va, size_t *);
                    bytes_write(self, z, sizeof(size_t));
                }
                break;
            case 'b':
                s=va_arg(va, char *);
                bytes_read(self, s, count);
                break;                
            case 's':
                for( ; count > 0 ; --count){
                    s=va_arg(va, char *);
                    size = strlen(s)+1;
                    bytes_read(self, s, size);
                }
                break;
            case 'S':
                for( ; count > 0 ; --count){
                    s=va_arg(va, char *);
                    size = strlen(s);
                    bytes_read(self, s, size);
                }
                break;
            case '.':
                c=0;
                for( ; count > 0 ; --count){
                    bytes_read(self, &c,1 );
                }
                break;
        }
        format++;
    }
    va_end(va);
}


/* File operations */
void bytes_save(Bytes self, const char * filename){
    FILE * fp = fopen(filename, "wb");
    fwrite(self->data,self->write_offset,1,fp);
    fflush(fp);
    fclose(fp);
}

Bytes bytes_load(const char * filename){
    unsigned char buffer[256];
    Bytes self = bytes_new();
    FILE * fp = fopen(filename, "rb");
    size_t chars_read=0;

    do{
        chars_read = fread(buffer, 1, 256 ,fp);
        bytes_write(self, buffer,chars_read);
    }while(chars_read == 256);

    fclose(fp);
    return self;
}





/*
* write to the memory location
* returns where the next element should be written
* example:   bwrite(array, &the_int, sizeof(int), 1)
*/
void * bwrite(void *memory, const void *ptr, size_t size, size_t nmemb){
    size_t total_size = size * nmemb;
    memcpy(memory, ptr, total_size);
    return (void *)(((char *)memory)+total_size);
}


/*
* read from memory location and sets ptr
* example:    bread(array, &the_int, sizeof(int), 1)
*/
void * bread(const void *memory, void *ptr, size_t size, size_t nmemb){
    size_t total_size = size * nmemb;
    memcpy(ptr, memory, total_size);
    return (void *)(((char *)memory)+total_size);
}





//~ int main(int argc, char **argv){
    //~ int i;
    //~ float f;
    //~ unsigned char C;

    //~ Bytes bs = bytes_new();

    //~ bytes_writef(bs, "Cif",'E', 5,2.5);
    //~ bytes_debug(bs);
    //~ bytes_readf(bs, "Cif", &C,&i,&f );
    //~ printf(" C:%c i:%i f:%f \n",C, i,f);

    //~ return 0;
//~ }



