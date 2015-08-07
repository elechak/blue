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
#ifndef _STACKS_
#define _STACKS_

#include <stdlib.h>


/* One step declaration and definition */
#define stack_create(NAME, TYPE, SIZE)\
struct NAME##_stack_struct{\
    size_t capacity;\
    size_t length;\
    TYPE *  data;\
} *NAME = malloc( sizeof(*NAME) );\
NAME->data = malloc( sizeof(TYPE) * SIZE );\
NAME->capacity = SIZE;\
NAME->length    =0;

/* Declare then define the stacks */
#define stack_define(ID, TYPE)\
typedef struct TYPE##_stack_struct{\
    size_t capacity;\
    size_t length;\
    TYPE *  data;\
} * ID;

#define stack_init(VAR, ID, SIZE)\
ID VAR = malloc( sizeof(*VAR));\
VAR->data  = malloc( sizeof(*(VAR->data)) * SIZE );\
VAR->capacity = SIZE;\
VAR->length    =0;


#define stack_new(VAR, ID, SIZE)\
VAR = malloc( sizeof(*(VAR)));\
VAR->data  = malloc( sizeof(*((VAR)->data)) * SIZE );\
VAR->capacity = SIZE;\
VAR->length    =0;


/* Common stack functionality */

#define stack_free(STACK)             (free(STACK->data))  ;  (free(STACK))
#define stack_pop(STACK)              (STACK->data[--(STACK->length)])
#define stack_dec(STACK)              --(STACK->length)
#define stack_length(STACK)           (STACK->length)
#define stack_peek(STACK)             (STACK->data[STACK->length-1])
#define stack_peekback(STACK, INDEX)  ((STACK)->data[(STACK)->length-(INDEX)-1])
#define stack_get(STACK, INDEX)       (STACK->data[INDEX])
#define stack_clear(STACK)            (STACK)->length = 0


#define stack_push(STACK, VAL)\
do{\
  if ((STACK)->length == (STACK)->capacity){\
    (STACK)->data = realloc((STACK)->data, sizeof(*((STACK)->data)) * ((STACK)->capacity + 64) );\
    (STACK)->capacity +=64;\
  }\
  (STACK)->data[(STACK)->length++]=(VAL);\
}while(0)

#define stack_shrink(STACK)\
do{\
    (STACK)->data = realloc((STACK)->data,  sizeof(*((STACK)->data))  * (STACK->length)  );\
    (STACK)->capacity = (STACK)->length;\
}while(0)

#define stack_iter(STACK, VAR, COUNT)\
if( (STACK)->length) for(COUNT=0; (COUNT < (STACK)->length) ? VAR=(STACK)->data[COUNT], 1 : 0; ++COUNT)

//if( (STACK)->length) for(COUNT=0; VAR=(STACK)->data[COUNT],COUNT < (STACK)->length; ++COUNT) 

#define stack_riter(STACK, VAR, COUNT)\
if( (STACK)->length) for(COUNT=(STACK)->length-1; (COUNT >= 0) ? VAR=(STACK)->data[COUNT], 1 : 0; --COUNT)

//if( (STACK)->length) for(COUNT=(STACK)->length-1; VAR=(STACK)->data[COUNT],COUNT>=0 ; --COUNT)


#define stack_append( STACK , STACK2)\
    (STACK)->capacity +=(STACK2)->length;\
    (STACK)->data = realloc((STACK)->data, sizeof(*((STACK)->data)) * ((STACK)->capacity) );\
    memcpy( ((STACK)->data) + ((STACK)->length), (STACK2)->data, sizeof(*((STACK)->data)) *((STACK2)->length) );\
    (STACK)->length+=(STACK2)->length;\



#endif
