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
#include "global.h"
#include "array.h"
#include "number.h"
#include "stack.h"


Link (*array_new)(size_t) = NULL;


/* LINK ARRAY */
LinkArray linkarray_new(size_t size){
    if ( size == 0 ) return NULL;
    LinkArray la = NULL;
    la = malloc( sizeof(LinkArray) + sizeof(Link) * size );
    la->size = size;
    for(;size ; size--){
        la->links[size-1] = NULL;
    }

    return la;
}

void linkarray_free( LinkArray la){
    if ( ! la) return;
    size_t delta;
    for(delta = 0; delta< la->size; delta++){
        if (la->links[delta] ) link_free(la->links[delta]);
    }
    free(la);
}



#define ARRAY_MIN_SIZE  8

static Link free_list = NULL;
static Mutex free_list_mutex;


static Mutex mutex_array_access[16];
static inline Mutex getArrayAccessMutex(Link self){
    return  mutex_array_access[ ((((unsigned long)self) & 0xF0)>>4) ];
}

static NativeType t;

static inline Array array_alloc(size_t length){    
    Array array = calloc( 1 , sizeof(struct Array) + sizeof(Link) *  ((length<ARRAY_MIN_SIZE)?ARRAY_MIN_SIZE:length) );
    array->length = length;
    return array;
}

static inline Array array_realloc(Array self, size_t length){
    if (length > ARRAY_MIN_SIZE) self = realloc( self, sizeof(struct Array) + sizeof(Link) * length );
    self->length = length;
    return self;
}

void array_size( Link self, int length){
    array_realloc( self->value.array,length);
}



Link array_new_threads(size_t length){
    Link link = NULL;
    
    mutex_lock(free_list_mutex);
    if ((free_list) && (length <=ARRAY_MIN_SIZE)){
        link = free_list;
        link->value.array->length = length;
        free_list = link->value.array->links[0];
        mutex_unlock(free_list_mutex);
    }else{
        mutex_unlock(free_list_mutex);
        link = object_create(t);
        link->value.array = array_alloc(length);
    }
    link->refcount = 1;
    return link;    
}

Link array_new_nothreads(size_t length){
    Link link = NULL;
    if ((free_list) && (length <=ARRAY_MIN_SIZE)){
        link = free_list;
        link->value.array->length = length;
        free_list = link->value.array->links[0];
    }else{
        link = object_create(t);
        link->value.array = array_alloc(length);
    }
    link->refcount = 1;
    return link;    
}

void array_destroy_threads(Link self){
    Array array = self->value.array;
    
    /* free all of the children contained in the array */
        int c;
        for (c =0 ; c < array->length ; c++) {
            if (array->links[c]){
                link_free( array->links[c]);
                array->links[c] = NULL;
            }
        }    
    
    /* clear the attributes */
        if (self->attribs) dictionary_clear(self->attribs);

    /* store array in free list if it is less than the minimum size */
        if (array->length <= ARRAY_MIN_SIZE){
            mutex_lock(free_list_mutex);
            array->links[0] = free_list;
            free_list = self;
            mutex_unlock(free_list_mutex);
            return;
        }
        
    /* free the object if not stored for reuse*/
        free(self->value.array);
        object_destroy(self);    
}

void array_destroy_nothreads(Link self){
    Array array = self->value.array;

    //fprintf(stderr, "destroying array %p\n", self);
    
    /* free all of the children contained in the array */
        int c;
        for (c =0 ; c < array->length ; c++) {
            if (array->links[c]){
                link_free( array->links[c]);
                array->links[c] = NULL;
            }
        }    
        
    /* clear the attributes */
        if (self->attribs) dictionary_clear(self->attribs);

    /* store array in free list if it is less than the minimum size */
        if (array->length <= ARRAY_MIN_SIZE){
            array->links[0] = free_list;
            free_list = self;
            return;
        }

    /* free the object if not stored for reuse*/
        free(self->value.array);
        object_destroy(self);    

}


void array_nullify(Link self){
    Link * links = self->value.array->links;
    size_t length = self->value.array->length;
    
    int c;
    for(c =0; c < length; c++){
        links[c] = NULL;
    }
}


size_t array_getLength(Link self){
    if (! self->value.array) return 0;
    return self->value.array->length;
}


Link array_new_resize(Link link,  int size, Link def){
    Array self = link->value.array;
    
    /* create array of requested size */
    Link thecopy_link = array_new(  size );
    if (!self) return thecopy_link;
    
    Array thecopy = thecopy_link->value.array;
    
    Mutex mutex = getArrayAccessMutex(link);
    mutex_lock(mutex);
    size_t orig_size  = self->length;
    
    int c;
    for(c=0; ((c < orig_size) && (c < size))   ; c++){
        thecopy->links[c] =  link_dup( self->links[c] );
    }
	
	if (def){
		for ( ;c<size;c++){
			thecopy->links[c] = link_dup(def);
		}
	}
    
    mutex_unlock(mutex);
    
    return thecopy_link;    
}


static Link array_new_slice(Link link,  int start, int end, Link def){
    Array self = link->value.array;
    
    /* create array of requested size */
    Link thecopy_link = array_new(  end-start );
    if (!self) return thecopy_link;
    
    Array thecopy = thecopy_link->value.array;
    
    Mutex mutex = getArrayAccessMutex(link);
    mutex_lock(mutex);
    size_t orig_size  = self->length;
    
	int i =0;
    int c;
    for(c=start; ((c < orig_size) && (c < end))   ; c++){
        thecopy->links[i++] =  link_dup( self->links[c] );
    }
	
	if (def){
		for ( ;c<end;c++){
			thecopy->links[i++] = link_dup(def);
		}
	}
    
    mutex_unlock(mutex);
    
    return thecopy_link;    
}





Link array_new_subarray( LinkStack source , size_t length){
    Link link  = array_new(length);
    Array array = link->value.array;
        
    if (length){
        memcpy(array->links, source->data + (source->length - length) , sizeof( Link )  * length);
        stack_length(source) -=length;
    }
        
    return link;
}


Link array_new_attach(LinkStack source){
    size_t length = stack_length(source);
    
    Link link  = array_new(  length );
    Array array = link->value.array;
    
    if (length){
        memcpy(array->links, source->data + (source->length - length) , sizeof( Link )  * length);
        stack_free(source);
    }
    
    return link;    
}


void array_set(Link self, int index, Link value){
    Array array = self->value.array;
    if (! array) return;
        
    Mutex mutex = getArrayAccessMutex(self);
    mutex_lock(mutex);
    Link old_value = array->links[index];
    array->links[index] = value; // child duplicated above
    mutex_unlock(mutex);
    if(old_value) link_free(old_value);    
};


void array_push(Link self, Link value){
    Array array = self->value.array;
    
    if ( ! array){
        array = self->value.array = array_alloc(1);
    }else{
        array = self->value.array = array_realloc(array,array->length+1);
    }
    
    array->links[array->length-1] = value;
}



static Link copy(Link self){
    Array source = self->value.array;
    
    if ( ! source) return array_new(0);
    
    Link dest_link = array_new(source->length);
    Array dest = dest_link->value.array;
    
    int c;

    for(c=0; c< source->length; c++){
        dest->links[c] =  link_dup(   source->links[c]  );
    }
    return dest_link;
}

Link * array_getArray(Link self){
    return self->value.array->links;
}

static int is_true(Link self){
    if (self->value.array->length) return 1;
    else return 0;
}


static Link addChild(Link self,Link child,Link key){
    Array array = self->value.array;
    Link * args = array_getArray(key);
    size_t argn  = array_getLength(key);    
    
    if (argn != 1) return NULL;
    if (args[0]->type !=Global->number_type)return NULL;
    
    long index =  (long) object_asNumber(args[0]);
    
    link_dup(child);
    
    Mutex mutex = getArrayAccessMutex(self);
    mutex_lock(mutex);
    Link old_child = array->links[index];
    array->links[index] = child; // child duplicated above
    mutex_unlock(mutex);
    if (old_child) link_free(old_child);
    return child;
}


static Link getChild(Link self,Link what){
    Array array    = self->value.array;
    if (( ! array)||(! array->length)) return NULL;
    
    Link * args = array_getArray(what);
    size_t argn  = array_getLength(what);    
    
    if (argn != 1) return NULL;
    if (args[0]->type !=Global->number_type)return NULL;    
    
    long index = (long) object_asNumber(args[0]);

    if (index >=array->length) return NULL;
        
    if (! array->links[index]) {
        array->links[index] = object_create(Global->null_type);
    }
    
    return link_dup( array->links[index]);
}

static Link array_merge(Link self, Link other){
    
    int length = self->value.array->length;
    
    Link newarray = array_new(length + other->value.array->length);
    
    Link * dest = newarray->value.array->links;
    Link * source = self->value.array->links;
    
    int c =0;
    for(; c < length; c++){
        dest[c] =  link_dup(  source[c]  );
    }
    
    dest = newarray->value.array->links + length;
    source = other->value.array->links;
    length = other->value.array->length;
    
    c =0;
    for(; c < length; c++){
        dest[c] =  link_dup(  source[c]  );
    }

    return newarray;
}


static Link op_plus(Link self, Link other){

    Array a1 = self->value.array;
    Array a2 = NULL;
    
    Link obj = NULL; // this is the return value

    /* adding array to array */
    if (other->type == t){ 
        if ( ! a1)  return copy(other);
    
        a2 = other->value.array;
        obj = array_new(a1->length + a2->length);
    }else{
        if ( ! a1){
            obj = array_new(1);
            obj->value.array->links[0] = link_dup(other);
            return obj;
        }

        obj = array_new(a1->length + 1);
    }

    Array a3 = obj->value.array;

    size_t c;
    size_t size;
    size_t index =0;

    size = a1->length;
    for(c=0; c< size; c++){
        a3->links[index++] = link_dup(a1->links[c]);
    }


    if (other->type == t){ // adding arrays together
        size = a2->length;
        for(c=0; c< size; c++){
            a3->links[index++] = link_dup(a2->links[c]);
        }
    } else{ // appending an object other than an array to an array
        a3->links[index++] = link_dup(other);
    }

    return obj;
}


static string_t asString(Link This){
    
    Array self = This->value.array;
    
    if (! self->length) return string_new("[]");
    
    string_t a = string_new("[");
    string_t sep = string_new(",");
    
    size_t c;
    Link link;
    
    for( c=0; c < self->length - 1 ; c++){
        link = self->links[c];
		if (link) a = string_new_merge(a , object_asString(link), 0,0);
		else     a = string_new_merge(a , string_new("()"), 0,0);
        a = string_new_merge(a,sep,0,1);
    }    
        
    link = self->links[c];
	if (link) a = string_new_merge(a , object_asString(link), 0,0);
	else     a = string_new_merge(a , string_new("()"), 0,0);
    a = string_new_merge(a , string_new("]"), 0,0);
    free(sep);

    return a;    
}


static NATIVECALL(array_map){
    Array self = This->value.array;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn <1) return exception("FunctionRequired",NULL,NULL);
        
    if (! self) return object_create(Global->null_type);
    
    size_t c;
    size_t size  = 0;
    size_t index = 0;

    /* create the new argument array */
    size = argn-1;
    Link newargs_link = array_new(argn-1);
    Array newargs = newargs_link->value.array;
    for(c=0; c< size; c++){
        newargs->links[index++] = link_dup(args[c+1]);
    }

    size = self->length;

    /* create the array that will be returned */
    Link rets_link = array_new(size);
    Array rets = rets_link->value.array;
    
    array_nullify( rets_link );
    
    Link link;
    int count =0;
    for( c=0; c<size; c++){
        /* call function and get return value */
        link = object_call(args[0],  self->links[c],newargs_link);
        
        if (link){
            if ( is_critical(link) ){
                link_free(rets_link);
                return link;                
            }
            rets->links[count++] = link;
            continue;
        }
    }

    /* resize array down to number of non null elements */
    array_realloc(rets, count);
    
    link_free(newargs_link);
    return rets_link;
};



static NATIVECALL(array_join){

    Array self = This->value.array;
    
    if (! self->length){
        return create_string_str(string_new(""));
    }
    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    size_t c;
    Link link;
    string_t a = string_new("");
    
    
    if (argn){
        string_t sep = object_asString(args[0]);
        for( c=0; c < self->length - 1 ; c++){
            link = self->links[c];
            a = string_new_merge(a , object_asString(link), 0,0);
            a = string_new_merge(a,sep,0,1);
        }    
        
        link = self->links[c];
        a = string_new_merge(a , object_asString(link), 0,0);
        free(sep);
        
    }else{
        for( c=0; c < self->length ; c++){
            link = self->links[c];
            a = string_new_merge(a , object_asString(link),0,0);
        }    
    }

    return create_string_str( a );
}

static NATIVECALL(array_length){
    Array self = This->value.array;
    if ( ! self) return  create_numberul( 0 );
    
    Mutex mutex = getArrayAccessMutex(This);
    mutex_lock(mutex);
    unsigned long length = self->length;
    mutex_unlock(mutex);
    return create_numberul(length);
}

void quicksort(Link self, int start, int end, Link cmp ){
  
    Link * array = self->value.array->links;
    
    Link temp;
    
    if (end > start + 1){
      
        Link piv = link_dup(array[start]); // firt element is the pivot
        int left = start + 1; 
        int right = end;
        
        int result;
        Link ret;
        Link zero    = create_numberi(0);     
        Link newargs = array_new(2);
             newargs->value.array->links[0] = zero;
             newargs->value.array->links[1] = piv;
        
        while (left < right){
            newargs->value.array->links[0] = array[left];
            ret = object_call(cmp,  self ,newargs);
            result = object_compare(ret, zero);
            link_free(ret);
            
            if ( result <= 0 ) left++;
            else{
              /* swap right with left, and decrement right index */
              right--;
              temp = array[left];
              array[left] = array[right];
              array[right] = temp;
            }
            
            newargs->value.array->links[0] = zero;
        }
        
        link_free(newargs);
        
        /* put the pivot in the correct position */
        left--;
        temp = array[left];
        array[left] = array[start];
        array[start] = temp;
        
        quicksort(self, start, left, cmp);
        quicksort(self, right, end, cmp);
    }
}

void quicksort_default(Link * array, int start, int end ){
  
    Link temp;
    
    if (end > start + 1){
      
        Link piv = array[start]; // firt element is the pivot
        int left = start + 1; 
        int right = end;
          
        while (left < right){
          if ( object_compare( array[left], piv ) <= 0 ) left++;
          else{
              /* swap right with left, and decrement right index */
              right--;
              temp = array[left];
              array[left] = array[right];
              array[right] = temp;
          }
        }
        /* put the pivot in the correct position */
        left--;
        temp = array[left];
        array[left] = array[start];
        array[start] = temp;
        
        quicksort_default(array, start, left);
        quicksort_default(array, right, end);
    }
}

static NATIVECALL(array_sort){
    Array self = This->value.array;
    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    Mutex mutex = getArrayAccessMutex(This);
    
    switch(argn){
        /* use default sort */
        case 0:
            mutex_lock(mutex);
            quicksort_default( self->links,0, self->length);
            mutex_unlock(mutex);
            return link_dup(This); 
        
        case 1:
            mutex_lock(mutex);
            quicksort( This,0, self->length, args[0]);
            mutex_unlock(mutex);
            return link_dup(This); 
        
        default:
            return exception("InvalidNumberOfArguments", NULL,NULL);        
    }
}

/* make a uniq copy of the original array */
static NATIVECALL(array_uniq){

    if ( This->value.array->length == 0 ) return array_new(0);
    else if ( This->value.array->length == 1 ) return copy(This);

    /* make a copy of the original array */    
    Link array_copy = copy( This );
    
    Array self = array_copy->value.array;
    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    switch(argn){
        /* use default sort */
        case 0:
            quicksort_default( self->links,0, self->length);
            break;
        case 1:
            quicksort( array_copy ,0, self->length, args[0]);        
            break;
        default:
            link_free(array_copy);
            return exception("InvalidNumberOfArguments", NULL,NULL);        
    }
    
    /* create a new array  */
    Link uniq = array_new( self->length );
    Link * uniq_links = uniq->value.array->links;
    
    
    int c = 1;
    int u_index = 1;
    Link last = self->links[0];
    uniq_links[0] = link_dup(self->links[0]);
    
    for(; c < self->length ; c++){
        if (last->type->compare){
            if (last->type->compare(last , self->links[c]) != 0){
                uniq_links[u_index++] = link_dup(self->links[c]);
                last = self->links[c];
            }
        }else{
            if (last != self->links[c] ){
                uniq_links[u_index++] = link_dup(self->links[c]);
                last = self->links[c];
            }
        }
    }
    
    link_free( array_copy );
    
    
    array_copy = array_new_resize( uniq, u_index,NULL );
    link_free(uniq);
    
    return array_copy;
}


static NATIVECALL(array_copy){
    return copy( This );
}

static NATIVECALL(array_resize){
    Link * args = array_getArray(Arg);
	size_t argn  = array_getLength(Arg);
	
	switch (argn){
		case 0:
			return link_dup(This);
			break;
		case 1:
			return array_new_resize(This,(size_t) object_asNumber(args[0]) , NULL);
			break;
		default:
			return array_new_resize(This,(size_t) object_asNumber(args[0]) , args[1]);
			break;
	}
    return link_dup(This);
}


static NATIVECALL(array_slice){
    Link * args = array_getArray(Arg);
	size_t argn  = array_getLength(Arg);
	
	switch (argn){
		case 0:
			return link_dup(This);
			break;
		case 1:
			return array_new_resize(This,(size_t) object_asNumber(args[0]) , NULL);
			break;
		case 2:
			return array_new_slice(This,(size_t) object_asNumber(args[0]) , (size_t) object_asNumber(args[1]), NULL);
			break;
		default:
			return array_new_slice(This,(size_t) object_asNumber(args[0]) , (size_t) object_asNumber(args[1]), args[2]);
			break;
	}
    return link_dup(This);
}




static NATIVECALL(array__append){ 
    return array_merge(This, Arg);
}

static NATIVECALL(array__merge){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    int c;
    
    Link a   = link_dup(This);
    Link b  = NULL;
    
    for( c = 0 ; c < argn; c++){
        if (args[c]->type != t) {
            link_free(a);
            return exception("ArgumentNotArray", NULL,NULL);     
        }
        b = array_merge(a, args[c]);
        link_free(a);
        a = b;
    }
    
    return b;
}


static NATIVECALL(array_contains){
    
    size_t argn  = array_getLength(Arg);
    
    if (argn != 1){
        return exception("ContainsTakesOneArgument", NULL, NULL);
    }
    
    Link link = array_getArray(Arg)[0];

    Link * items = array_getArray( This );
    size_t num_items = array_getLength( This );
    
    int c = 0;
    
    if (link->type->compare){ 
        for( ; c <num_items; c++){
           if (( link == items[c] ) || ( link->type->compare(link, items[c]) == 0 ))  return create_numberi(1);
        }
    }else{
        for( ; c <num_items; c++){
            if ( link == items[c] ) return create_numberi(1);
        }        
    }
    return create_numberi(0);
}


static NATIVECALL(array_asAttribs){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    if (! argn) return create_null();
    
    Link link = link_dup(args[0]);
    
    Link * items = array_getArray( This );
    size_t items_size  = array_getLength( This );
    
    Link * keys = array_getArray(args[1]);
    argn = array_getLength(args[1]);
    
    int c;
    for ( c=0; c<argn; c++){
        if( c < items_size) addAttr( link ,items[c] ,keys[c]); // obj, value, key
        else addAttr( link ,create_null() ,keys[c]); // obj, value, key
    }
    
    return link;
}


NativeType array_init(){
    free_list = NULL;
    free_list_mutex = mutex_new();
    array_new = array_new_nothreads;
    int c;
    for (c =0; c < 16; c++){
        mutex_array_access[c] = mutex_new();
    }
    
    t = newNative();
        t->is_true        = is_true;
        t->getChild       = getChild;
        t->addChild       = addChild;
        t->destroy        = array_destroy_nothreads;
        t->copy           = copy;
        t->op_plus        = op_plus;
        t->asString       = asString;

    addNativeCall(t, "print", universal_print);
    addNativeCall(t, "contains", array_contains);
    addNativeCall(t, "append", array__append);
    addNativeCall(t, "merge", array__merge);
    addNativeCall(t, "map", array_map);
    addNativeCall(t, "join", array_join);
    addNativeCall(t, "length", array_length);
    addNativeCall(t, "resize", array_resize);
    addNativeCall(t, "sort", array_sort);
    addNativeCall(t, "copy", array_copy);
    addNativeCall(t, "uniq", array_uniq);
    addNativeCall(t, "attrib", array_asAttribs);
    addNativeCall(t, "slice", array_slice);

    return t;
}


