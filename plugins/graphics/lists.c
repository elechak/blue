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

#include "lists.h"

#define LASTPOINTER                      list->data+(list->length * list->element_size)
#define INDEX2POINTER(INDEX)         list->data+((INDEX) * list->element_size)
#define INDEX_LIMIT(LIST,INDEX)     if (INDEX >= LIST->length) INDEX=LIST->length-1;\
                                                    if (INDEX <0){\
                                                        INDEX = LIST->length + INDEX;\
                                                        if (INDEX <0){\
                                                            INDEX=0;\
                                                        }\
                                                  }
#define INDEX_SLICE_LIMIT(LIST,INDEX)         if (INDEX > LIST->length) INDEX=LIST->length;\
                                                                if (INDEX <0){\
                                                                    INDEX = LIST->length + INDEX;\
                                                                    if (INDEX <0){\
                                                                        INDEX=0;\
                                                                    }\
                                                                }

#define GROW_RATE 8


// CREATION functions

// allocate memory for list
// size is how many chunks of element_size bytes
int list_alloc(List list, int s){
    list->data = realloc(list->data, s * list->element_size );
    if (! list->data){
        list->size = 0;
        return 0;
    }
    list->size = s;
    return s;
}

// create a new list, it does not allocate any memory to storage yet
// es is the element_size normaly you would use sizeof(dodad) where
// dodad is the thing you want to store
List list_new(int es){
    List list;
    list = malloc( sizeof(struct List) );
    if (! list) return NULL;
    list->data = NULL;
    list->size=0;
    list->length =0;
    list->element_size = es;
    list_alloc(list,1);
    return list;
}


void list_dump(List list, char * fname){
    FILE * fp;
    fp = fopen(fname, "wb");
    fwrite(&(list->size),sizeof(int),1,fp);
    fwrite(&(list->length),sizeof(int),1,fp);
    fwrite(&(list->element_size),sizeof(int),1,fp);
    fwrite(list->data,list->length * list->element_size,1,fp);
    fclose(fp);
}


List list_load(char * fname){
    FILE * fp;
    List list;
    list = list_new(1);
    fp = fopen(fname, "rb");
    fread(&(list->size),sizeof(int),1,fp);
    fread(&(list->length),sizeof(int),1,fp);
    fread(&(list->element_size),sizeof(int),1,fp);
    list_alloc(list,list->length);
    fread(list->data,list->length * list->element_size,1,fp);
    fclose(fp);
    return list;
}

// deletes the list , don't use this list again
void list_free(List list){
    if (list->data){
        free(list->data);
        list->data = NULL;
    }
    free(list);
}

// removes all elements in the list
void list_clear(List list){
    list->length = 0;
}

//loads a text file
List list_fromFile(char * fname){
    List list;
    FILE * fp;
    int size;

    fp = fopen(fname, "rb");
    if ( ! fp ){
        return NULL;
    }

    list=list_new(1);
    if (! list){
        fclose(fp);
        return NULL;
    }

    fseek(fp,0,SEEK_END);
    size = ftell(fp);
    size = list_alloc(list,size);
    list->length = size;
    fseek(fp,0,SEEK_SET);
    fread(list->data,size,1,fp);
    fclose(fp);
    return list;
}

List list_loadfromFile(List list, char * fname){
    FILE * fp;
    int size;

    fp = fopen(fname, "rb");
    if ( ! fp ){
        return NULL;
    }

    if(list){
        list_clear(list);
    }else{
        list=list_new(1);
        if (! list){
            fclose(fp);
            return NULL;
        }
    }

    fseek(fp,0,SEEK_END);
    size = ftell(fp);
    size = list_alloc(list,size);
    list->length = size;
    fseek(fp,0,SEEK_SET);
    fread(list->data,size,1,fp);
    fclose(fp);
    return list;
}


// Create a list from a null-terminated string
List list_fromString(char * string){
    List list;
    int size;
    list = list_new(1);
    size = strlen(string)+1;
    list_alloc(list ,size);
    memcpy(list->data , string, size); // copies the null character as well
    list->length = size;
    return list;
}

// Copy the string to the list
List list_setString(List list, char * string){
    int size;
    list->element_size = 1;
    size = strlen(string)+1;
    list_alloc(list ,size);
    memcpy(list->data , string, size);
    list->length = size;
    return list;
}

// Create a list (of ints) from a array of ints
List list_fromIntArray(int * array, int size){
    List list;
    list = list_new(sizeof(int));
    list_alloc(list ,size * sizeof(int));
    memcpy(list->data , array, size*sizeof(int));
    list->length = size;
    return list;
}

// Create a list (of floats) from an array of floats
List list_fromFloatArray(float * array, int size){
    List list;
    list = list_new(sizeof(float));
    list_alloc(list ,size * sizeof(float));
    memcpy(list->data , array, size*sizeof(float));
    list->length = size;
    return list;
}

// Return the element at the index
void * list_get(List list, int index){
    INDEX_LIMIT(list,index)
    return INDEX2POINTER(index);
}

void list_set(List list, int index, void * data){
    void * where;
    where = list_get(list,index);
    memcpy(where,data,list->element_size);
}

// Delete element at index
void list_delete(List list, int index){
    if (list->length == 0) return;
    INDEX_LIMIT(list,index)
    if (index == (list->length -1)){
        list->length--;
        return;
    }
    memmove(INDEX2POINTER(index), INDEX2POINTER(index+1), list->element_size * (list->length - index));
    list->length--;
}


long list_find(const List list, const void * value){
    register long iter_index, length;
    int element_size = list->element_size;
    int c;

    char *element, *v;

    length = list->length;
    for (iter_index = 0 ; iter_index < length ; iter_index++){
        element = list->data+((iter_index) * element_size);
        c = element_size;
        v = (char *) value;
        while(  c-- && *element++ == *v++ );
        if (c==-1) return iter_index;
    }
    return -1;
}

void list_remove(List list, const void * value){
    int index;

    index = list_find(list,value);
    if (index ==-1) return;
    list_delete(list, index);
}


//Append element to end of list
void list_append(List list , void * value){
    if (list->length == list->size){
        list_alloc(list , list->length + GROW_RATE);
    }
    memcpy( LASTPOINTER , value, list->element_size);
    list->length++;
}


//Insert an element into the list at the index
// the element will end up at the index value specified
void list_insert(List list, int index, void * value){
    void *p;

    // handle index greater than length as an append
    if (index >= list->length){
        list_append(list,value);
        return;
    }

    // handle negative index
    if (index <0){
        index = list->length + index+1;
        if (index <0) index = 0;
    }

    p = INDEX2POINTER(index);

    //fprintf(stderr,"before index=%p  data=%p size=%i length=%i\n", p, list->data, list->size, list->length);
    if (list->length == list->size) list_alloc(list , list->length + GROW_RATE);
    p = INDEX2POINTER(index);

    memmove(INDEX2POINTER(index+1), p, list->element_size * (list->length - index));
    memcpy(p , value, list->element_size);
    list->length++;
}


//Append all of the data from the other list to the list in the first argument
void list_extend(List list, List other){
    int size_diff;
    if (list->element_size != other->element_size){
        fprintf(stderr,"lists contain different types\n");
        return;
    }

    size_diff = other->length - (list->size - list->length);
    if (size_diff > 0 ) list_alloc(list, list->size + size_diff);

    memcpy(LASTPOINTER, other->data, other->length * other->element_size);
    list->length += other->length;
}

//return a new list consisting of the elements between start and end skipping skip elements
List list_skipSlice(List list,int start, int end,int skip){
    List newarray;
    newarray = list_new(list->element_size);
    INDEX_SLICE_LIMIT(list,start)
    INDEX_SLICE_LIMIT(list,end)
    if (start <end){
        for (;start < end; start += skip) list_append(newarray, list_get(list,start));
    }else{
        for (;start > end; start -= skip) list_append(newarray, list_get(list,start));
    }
    return newarray;
}

// Create a copy of the source list  allocates new list if dest is NULL else it copies source into dest
List list_copy(List source, List dest){

    if (! dest ){
        dest = list_new(source->element_size);
        if (! dest ) return NULL;
    }

    list_clear(dest);
    list_extend(dest, source);
    return dest;
}


void list_pack(List source, int index){
    INDEX_LIMIT(source, index);
    memmove(source->data,
        source->data+(index *source->element_size),
        (source->length - index)*source->element_size);
    source->length -=  index;
}

int list_eq(List a, List b){
    if (! (a->length ==b->length))return 0;
    if( *(a->data) != *(b->data)) return 0;
    if ( memcmp(a->data, b->data, a->length * a->element_size)) return 0;
    return 1;
}

//--------------------------------TEXT OPERATIONS------------------------------------------------------


int list_streq(List l , char * string){
    if(strlen(string) != l->length) return 0;
    if( *(l->data) != *string) return 0;
    if( memcmp(l->data,string,l->length)==0 )return 1;
    return 0;
}

void list_strcat(List list, char * source){
    int len;
    len = strlen(source);

    if ((len + list->length) > list->size)
        list_alloc(list , (len + list->length));

    if (list->length){
        memcpy((LASTPOINTER)-1,source, len+1);
        list->length +=len;
    }else
        list_setString(list, source);
}


//--------------------- Memory Operation-------------------------------------------------------------------------
List list_memset(List list, char * source, int length){
    list->element_size = 1;
    list_alloc(list ,length);
    memcpy(list->data , source, length);
    list->length = length;
    return list;
}


void list_memcat(List list, char * source, int length){

    if ((length + list->length) > list->size)
        list_alloc(list , (length + list->length));

    if (list->length){
        memcpy((LASTPOINTER),source, length);
        list->length +=length;
    }else
        list_memset(list, source, length);
}


//--------------------------------  MATRIX OPERATIONS------------------------------------------------


//~ float list_vecMult(List m , List v){
    //~ float x;
    //~ float sum = 0.0;
    //~ list_iter(m,float,x)sum += x * list_value(v,float,m->iter);
    //~ return sum;
//~ }


//~ void list_normalize(List lst){
    //~ float mag, *a, sum,val;
    //~ sum = 0;
    //~ list_iter(lst,float,val ){
        //~ sum+=val * val;
    //~ }
    //~ mag = sqrt(sum);
    //~ list_piter(lst,a) *a/=mag;
//~ }

//~ List list_matMult(List m1, List m2){
    //~ List res, v1,v2;
    //~ float ans;
    //~ int m1_rows, m1_cols, m2_cols,r,c;
    //~ m1_cols = m1->order;
    //~ m1_rows = m1->length/m1_cols;
    //~ m2_cols = m2->length/m1_cols;

    //~ res = list_new(sizeof(float));
    //~ for (r=0; r< m1_rows ; r++){
        //~ v1 = list_getRow(m1,r);
        //~ for (c=0; c< m2_cols ; c++){
            //~ v2 = list_getCol(m2,c);
            //~ ans = list_vecMult(v1,v2);
            //~ list_append(res, &ans);
            //~ list_free(v2);
        //~ }
        //~ list_free(v1);
    //~ }
    //~ res->order=m2->order;
    //~ return res;
//~ }











