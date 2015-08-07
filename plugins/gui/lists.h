/*
-------------------------------------------
Copyright 2005 Erik Lechak
-------------------------------------------


This file is part of bat.

Bat is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Bat is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bat; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef _LIST
#define _LIST
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../library_header.h"

//-----------------TEXT MACROS-----------------------------------------------------
#define STRING( VAR , S) List VAR = list_fromString(S)
#define print(LIST)   list_print(LIST,char,"%c")
#define cat(LIST, OTHER) list_extend(LIST,OTHER)

//-------------------------- LIST MACROS---------------------------------------------

#define list_getLength(LIST) (LIST)->length

//iterate through all values (deref pointers and cast to TYPE)
#define list_iter(LIST,TYPE, VAR)   if (LIST && LIST->length) for(iter_index=0 ; VAR = *((TYPE *)list_get(LIST,iter_index)) , iter_index<LIST->length ; iter_index++)

//iterate through all pointers
#define list_piter(LIST,VAR)   if (LIST && LIST->length) for(iter_index=0 ; VAR = list_get(LIST,iter_index) , iter_index<LIST->length ; iter_index++)
#define list_piter_nr(LIST,VAR)   if (LIST && LIST->length) for(; VAR = list_get(LIST,iter_index) , iter_index<LIST->length ; iter_index++)

//iterate through pointers starting at index start until end
#define list_bpiter(LIST,VAR,START,END)   \
if (LIST && LIST->length) for(iter_index=START ; VAR = list_get(LIST,iter_index) , iter_index<END ; iter_index++)

//get single value and cast to TYPE
#define list_value(LIST,TYPE, INDEX)   (*((TYPE *)list_get(LIST,INDEX)))

// print the list provided the type to cast it to then the format
#define list_print(LIST,TYPE,FORMAT)   if (LIST && LIST->length)for(iter_index=0 ;  iter_index< LIST->length ; iter_index++)  {printf(FORMAT, *((TYPE *)list_get(LIST,iter_index)) );}iter_index=0;

//returns new list of the resulting slice
#define list_slice(LIST,START,END) list_skipSlice(LIST,START,END,1)

#define list_pop(LIST) if (LIST && LIST->length){ LIST->length--;}


struct List{
    char * data; // Note that this is the first element of this structure
    size_t size;     // how many elements this list can hold without memory allocation
    size_t length; // how many elements currently reside in this list
    size_t element_size; // size of an individual element in bytes
};

typedef struct List * List;


//--------------------List Constructors------------------------------------------

//Allocate memory for list -- used internally
int list_alloc(List list, int size);

//Create a new list
EXPORT List list_new(int element_size);

//Create a list from a null terminated string
EXPORT List list_fromString(char * string);
EXPORT List list_loadfromFile(List list, char * fname);
// create a list from a C array of ints
EXPORT List list_fromIntArray(int * array, int size);

// create a list from a C array of floats
EXPORT List list_fromFloatArray(float * array, int size);

// Create a list (of chars) from a text file
EXPORT List list_fromFile(char * fname);

//------------------------Saving and Loading-----------------------------------------------------
//Save the list to a file
EXPORT void list_dump(List list, char * fname);

//Load the list from a file
EXPORT List list_load(char * fname);

// Free memory and destroy list
EXPORT void list_free(List list);

//Get the pointer to the data at the specified index
EXPORT void * list_get(List list, int index);

EXPORT void list_set(List list, int index, void * data);
EXPORT void list_delete(List list, int index);
EXPORT void list_remove(List list, const void * value);
EXPORT void list_clear(List list);
EXPORT void list_insert(List list, int index, void * value);
EXPORT void list_append(List list , void * value);
EXPORT void list_extend(List list, List other);
EXPORT List list_copy(List source, List dest); // Create a copy of the list
EXPORT List list_skipSlice(List list,int start, int end,int skip);
EXPORT long list_find(const List list, const void * value); //Find first occurance of value in list
EXPORT void list_pack(List source, int index);


//Returns 1 if lists a and b contain the same data
EXPORT int list_eq(List a, List b);


//---------------------Text Operations---------------------------------------------------------------------------

//Returns 1 if list l matches the null terminated string
EXPORT int list_streq(List l , char * string);
EXPORT List list_setString(List list, char * string);
EXPORT void list_strcat(List dest, char * source);

//--------------------- Memory Operation-------------------------------------------------------------------------
EXPORT List list_memset(List list, char * source, int length);
EXPORT void list_memcat(List list, char * source, int length);

//--------------------- Matrix Operation-------------------------------------------------------------------------

//~ #define list_getRow(LIST, INDEX) list_slice(LIST, INDEX * LIST->order, (INDEX+1) * LIST->order)
//~ #define list_getCol(LIST, INDEX) list_skipSlice(LIST, INDEX, LIST->length, LIST->order)

//~ float list_vecMult(List m , List v);
//~ List list_matMult(List m1, List m2);
//~ void list_normalize(List lst);


#endif


