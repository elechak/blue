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

#include <math.h>
#include "number.h"



static NativeType t;

static Link  free_list;
static Mutex free_list_mutex;

void number_destroy_nothreads(Link self){
    if (self->attribs) dictionary_free(self->attribs);
    self->value.link = free_list;
    free_list = self;
}

void number_destroy_threads(Link self){
    if (self->attribs) dictionary_free(self->attribs);
    mutex_lock(free_list_mutex);
    self->value.link = free_list;
    free_list = self;
    mutex_unlock(free_list_mutex);
}


Link number_new_nothreads(){
    Link link;
    if (free_list){
        link = free_list;
        free_list = link->value.link;
    }else{
        link = object_create(t);
    }
    link->refcount  = 1;
    return link;
}

Link number_new_threads(){
    Link link;
    mutex_lock(free_list_mutex);
    if (free_list){
        link = free_list;
        free_list = link->value.link;
        mutex_unlock(free_list_mutex);
    }else{
        mutex_unlock(free_list_mutex);
        link = object_create(t);
    }
    link->refcount  = 1;
    return link;
}



#define RETURN_NUMBER(VAR)\
    Link link = number_new();\
    link->value.number = VAR;\
    return link;

Link create_number(number_t x){ 
    RETURN_NUMBER(x);
}

Link create_numberd(double x){
    RETURN_NUMBER(x);
}

Link create_numberi(int x){
    RETURN_NUMBER(x);
}

Link create_numberul(unsigned long x){
    RETURN_NUMBER(x);
}

static Link copy(Link self){
    return create_number( self->value.number);
}

static number_t asNumber(Link self){
    return self->value.number;
}

static string_t asString(Link self){
    string_t s = string_new_formatted("%f", self->value.number );
    
    char * c = s->data+s->length-1;

    while( *c == '0'){
        *c=0;
        c--;
        s->length--;
    }

    if ((*c=='.') && (*(c+1)==0)){
        *c=0;
        s->length--;
    }

    return s;
}


static int is_true(Link self){
    return self->value.number ? 1: 0;
}

static int compare(Link self, Link other){
    number_t a = self->value.number;
    number_t b = object_asNumber(other);
    return a>b  ? 1  : a<b  ? -1 : 0;
}

/*  */
static Link op_plus(Link self, Link other){
    if (other->type != t) return create_number(self->value.number + object_asNumber(other)  );
    return create_number(self->value.number + other->value.number  );
}

static Link op_minus(Link self, Link other){
    if (other->type != t) return create_number(self->value.number - object_asNumber(other)  );
    return create_number(self->value.number - other->value.number  );
}

static Link op_multiply(Link self, Link other){
    if (other->type != t) return create_number(self->value.number * object_asNumber(other)  );
    return create_number(self->value.number * other->value.number  );
}

static Link op_divide(Link self, Link other){
    if (other->type != t) return create_number(self->value.number / object_asNumber(other)  );
    return create_number(self->value.number / other->value.number  );
}

static Link op_modulus(Link self, Link other){
    if (other->type != t) return create_number(   fmod( self->value.number , object_asNumber(other))  );
    return create_number(   fmod(self->value.number , other->value.number)  );
}

static Link op_power(Link self, Link other){
    if (other->type != t) return create_number(   pow( self->value.number , object_asNumber(other))  );
    return create_number(   pow(self->value.number , other->value.number)  );
}

static Link op_neg(Link self){
    return create_number( self->value.number  * -1  );
}

static NATIVECALL(num_abs){
    return create_number( (number_t)fabs( This->value.number ));
}

double trunc(double);
static NATIVECALL(num_int){
    return create_numberd( trunc((double)This->value.number) );
}

static NATIVECALL(num_frac){
    return create_numberd( (double)This->value.number -  trunc((double)This->value.number) );
}


static NATIVECALL(num_str){
    return create_string_str( asString( This ) );
}

double round(double);
static NATIVECALL(num_round){
    return create_numberd( round( (double)This->value.number ) );
}

static NATIVECALL(num_sin){
    return create_number( (number_t) sin(This->value.number) );
}

static NATIVECALL(num_cos){
    return create_number( (number_t) cos(This->value.number) );
}

static NATIVECALL(num_tan){
    return create_number( (number_t) tan(This->value.number) );
}

static NATIVECALL(num_asin){
    return create_number( (number_t) asin(This->value.number) );
}

static NATIVECALL(num_atan){
    return create_number( (number_t) atan(This->value.number) );
}

static NATIVECALL(num_log){
    return create_number( (number_t) log(This->value.number) );
}

static NATIVECALL(num_log10){
    return create_number( (number_t) log10(This->value.number) );
}


static NATIVECALL(num_xor){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    long a,b;
    switch( argn ){
        case 0:
            return link_dup(This);
            break;
        default:
            a = (long)This->value.number;
            b = (long)object_asNumber(args[0]);
            return create_numberi(a^b);
    }
}

static NATIVECALL(num_or){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    long a,b;
    switch( argn ){
        case 0:
            return link_dup(This);
            break;
        default:
            a = (long)This->value.number;
            b = (long)object_asNumber(args[0]);
            return create_numberi(a | b);
    }
}

static NATIVECALL(num_and){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    long a,b;
    switch( argn ){
        case 0:
            return link_dup(This);
            break;
        default:
            a = (long)This->value.number;
            b = (long)object_asNumber(args[0]);
            return create_numberi(a & b);
    }
}


static NATIVECALL(num_shiftleft){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    long a,b;
    switch( argn ){
        case 0:
            return link_dup(This);
            break;
        default:
            a = (long)This->value.number;
            b = (long)object_asNumber(args[0]);
            return create_numberi(a << b);
    }
}

static NATIVECALL(num_shiftright){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    long a,b;
    switch( argn ){
        case 0:
            return link_dup(This);
            break;
        default:
            a = (long)This->value.number;
            b = (long)object_asNumber(args[0]);
            return create_numberi(a >> b);
    }
}


NativeType number_init(){
    free_list = NULL;
    free_list_mutex = mutex_new();
    
    number_new = number_new_nothreads;
    
    t = newNative();
        t->destroy        = number_destroy_nothreads;
        t->copy           = copy;
        t->asNumber       = asNumber;
        t->asString       = asString;
        t->op_plus        = op_plus;
        t->op_minus       = op_minus;
        t->op_multiply    = op_multiply;
        t->op_divide      = op_divide;
        t->op_modulus     = op_modulus;
        t->op_power       = op_power;
    
        t->op_neg         = op_neg;
        t->is_true        = is_true;
        t->compare        = compare;
    
    addNativeCall(t, "print", universal_print);
    addNativeCall(t, "str", num_str);
    addNativeCall(t, "abs", num_abs);
    addNativeCall(t, "frac", num_frac);
    addNativeCall(t, "int", num_int);
    addNativeCall(t, "round", num_round);
    addNativeCall(t, "sin", num_sin);
    addNativeCall(t, "cos", num_cos);
    addNativeCall(t, "tan", num_tan);
    addNativeCall(t, "asin", num_asin);
    addNativeCall(t, "atan", num_atan);
    addNativeCall(t, "log", num_log);
    addNativeCall(t, "log10", num_log10);
    addNativeCall(t, "band", num_and);
    addNativeCall(t, "bor", num_or);
    addNativeCall(t, "bxor", num_xor);
    addNativeCall(t, "shiftright", num_shiftright);
    addNativeCall(t, "shiftleft", num_shiftleft);
    
    return t;
}


