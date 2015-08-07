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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "skiparray.h"


static SkipArrayBlock skiparray_newblock(size_t capacity);

struct SkipArrayBlock{
    size_t length; // number of elements in this block
    size_t capacity; // capacity of this block
    void ** data;
    SkipArrayBlock next;
};

static SkipArrayBlock skiparray_newblock(size_t capacity){
    SkipArrayBlock self = malloc(sizeof(*self));
    self->length   = 0;
    self->capacity = capacity;
    self->data     = calloc( capacity, sizeof(void *) );
    self->next     = NULL;
    return self;
}

SkipArray skiparray_new(){
    SkipArray self = malloc(sizeof(*self));
    self->length = 0;
    self->blocksize = 32;
    self->root = skiparray_newblock(self->blocksize);
    self->destroyer = NULL;
    return self;
}

void skiparray_setDestroyer(SkipArray self , Destroyer d){
    self->destroyer = d;
}

void skiparray_free(SkipArray self){
    SkipArrayBlock temp = NULL;
    SkipArrayBlock block = self->root;
    size_t length ;
    size_t index;

    while(1){
        temp = block->next;

        if (block->data){

            /* free all the pointers contained in the block */
            if (self->destroyer){
                length = block->length;
                for (index = 0; index < length; index++){
                    self->destroyer(block->data[index]);
                }
            }
            free(block->data);
        }

        free(block);
        if (temp) block = temp;
        else break;
    }
    free(self);
}


void * skiparray_get(SkipArray self, size_t index){

    if (index >= self->length) return NULL;

    size_t offset = 0;
    SkipArrayBlock block = self->root;

    while(1){
        if (index < block->length + offset){
            return block->data[index - offset];
        }else{
            offset += block->length;
            if (! block->next) return NULL;
            block = block->next;
        }
    }
}


size_t skiparray_find(SkipArray self, void * key , Comparator comp){

    if ((! key) || (! comp)) return self->length;

    size_t index        = 0;
    size_t offset       = 0;
    SkipArrayBlock block = self->root;

    while(1){

        if (index < block->length + offset){
            if ( 0 == comp(key, block->data[index-offset] ) ) return index;
            index++;
        }else{
            offset += block->length;
            if (! block->next) return self->length;
            block = block->next;
        }
    }
}

void skiparray_append(SkipArray self, void * data){
    skiparray_insert(self, self->length, data);
}



static void skiparray_splitblock(SkipArray sa, SkipArrayBlock a){
    //~ int c,t;
    size_t asize, bsize;
    SkipArrayBlock b = skiparray_newblock(sa->blocksize);
    asize = a->length/2;
    bsize = a->length - asize;

    memcpy(b->data, a->data + asize, bsize * sizeof(void *));
    a->length = asize;
    b->length = bsize;
    b->next = a->next;
    a->next = b;
}

void skiparray_insert(SkipArray self, size_t index, void * data){
    // self          - this skiparray
    // index      - the index you are looking for
    // data        - the pointer you are inserting into the skiparray

    size_t eindex; // the index with respect to the current block
    size_t offset; // the number of elements in all previous blocks
    SkipArrayBlock block = NULL;  //current block

    start:
    eindex = 0;
    offset = 0;
    block = self->root;

    while(1){
        eindex = index - offset; // the index with respect to the current block
        if ((eindex <= block->length) && (eindex < block->capacity)){
            // this index should be inserted into this block
            break;
        }else{

            if (! block->next){
                if(eindex < block->capacity){
                    // Found correct element
                    // Need to add padding
                    self->length += -block->length;
                    block->length = eindex;
                    self->length += block->length;
                    break;
                }else{
                    self->length += block->capacity - block->length;
                    block->length = block->capacity;
                    block->next = skiparray_newblock(self->blocksize);
                }
            }
            offset += block->length;
            block = block->next;
        }
    }

    eindex = index - offset;

    //printf(" i=%i, offset=%i, capac=%i, len=%i\n",index, offset,block->capacity, block->length);


    if (block->length == block->capacity){
        /* there is no more room in this block to hold this data  */
        /* need to split this block */
        skiparray_splitblock(self, block);
        goto start;
    }


    if (eindex < block->length){
        memmove(block->data + eindex+1, block->data + eindex, (block->length - eindex)* sizeof(void **));
        block->data[eindex] = data;
        block->length++;
        self->length++;
    }else if (eindex == block->length){
        self->length++;
        block->length++;
        block->data[eindex] = data;
    }else{
        printf("SKIPARRAY INSERT ERROR\n");
        exit(1);
    }
}

void * skiparray_delete(SkipArray self, size_t index){
    void * ret = NULL;
    size_t eindex;
    if (index >= self->length) return NULL;

    size_t offset = 0;
    SkipArrayBlock block = self->root;

    while(1){
        eindex = index - offset; // the index with respect to the current block

        // check if eindex is within the scope of this block
        if ((eindex <= block->length) && (eindex < block->capacity)){

            // this is the element we want
            ret = block->data[eindex];

            // slide the other entries into the deleted entries position
            if (eindex < block->length-1)
                memmove(block->data + eindex, block->data + (eindex+1), (block->length - eindex-1)* sizeof(void *));

            //decrement the length of this block and the total length
            block->length--;
            self->length--;
            return ret;
        }else{
            // move to next block
            offset += block->length;
            block = block->next;
        }
    }
}



void * skiparray_replace(SkipArray self, size_t index, void * data){
    void * ret = NULL;
    size_t eindex;
    if (index >= self->length) return NULL;

    size_t offset = 0;
    SkipArrayBlock block = self->root;

    while(1){
        eindex = index - offset; // the index with respect to the current block

        // check if eindex is within the scope of this block
        if ((eindex <= block->length) && (eindex < block->capacity)){

            // this is the element we want

            // clean up the data that is in the indexed position
            ret = block->data[eindex];

            // put the new data into that position
            block->data[eindex] = data;

            return ret;
        }else{
            // move to next block
            offset += block->length;
            block = block->next;
        }
    }
}






