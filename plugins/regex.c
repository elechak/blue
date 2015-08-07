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

#include "../global.h"
#include <regex.h>


EXPORT void init(INITFUNC_ARGS);

static NativeType regex_type;


struct blue_regex{
    regex_t re;
    string_t pattern;
};
typedef struct blue_regex * blue_regex;


static void  create(Link self){
    self->value.vptr = malloc( sizeof( struct blue_regex ));
}

static void destroy(Link self){    
    blue_regex br = self->value.vptr;
    if (br) {
        regfree( &br->re );
        free( br->pattern);
        free(br);
    }
    object_destroy(self);   
}

static NATIVECALL(regex_pattern){    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    if ( argn != 1 ) return exception("ArgError", NULL,NULL);
    
    string_t s = object_getString( args[0]);
    
    if ( ! s) return exception("ArgError", NULL,NULL);
    
    Link link = object_create( regex_type );
    
    blue_regex br = link->value.vptr;
    br->pattern = string_dup(s);
    
    int err = regcomp( &(br->re), s->data, REG_EXTENDED);
    
    if (err){
        link_free(link);
        return exception("RegexPatternError", NULL,NULL);
    }
    
    return link;
}

static string_t regex_asString(Link self){    
    blue_regex br = self->value.vptr;
    return string_dup(br->pattern);
}


static NATIVECALL(regex_match){ 
    blue_regex br = This->value.vptr;
    regmatch_t match[64];
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    if ( argn != 1 ) return exception("ArgError", NULL,NULL);
    
    string_t s = object_getString( args[0]);
    
    if ( ! s) return exception("ArgError", NULL,NULL);    
    
    regexec(&(br->re),s->data,64,match,0); 
    
    Link a = array_new(0);

    string_t mm;
    int length;
    int count = 0;
    while ((count < 64)&&( match[count].rm_so != -1)){
        //DPRINT(" %i %i\n",  match[count].rm_eo , match[count].rm_so);
        length = match[count].rm_eo - match[count].rm_so;
        mm = string_new_ls(length, NULL);
        memcpy(mm->data, s->data + match[count].rm_so, length);
        array_push(a, create_string_str( mm) );
        count++;
    }
    
    return a;
}

static NATIVECALL(regex_exec){ 
    blue_regex br = This->value.vptr;
    regmatch_t match[64];
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if ( argn != 1 ) return exception("ArgError", NULL,NULL);

    string_t s = object_getString( args[0]);

    if ( ! s) return exception("ArgError", NULL,NULL);    

    regexec(&(br->re),s->data,64,match,0); 

    Link a = array_new(0);
    Link b;

    int count = 0;
    while ((count < 64)&&( match[count].rm_so != -1)){

        b = array_new(2);
        b->value.array->links[0] = create_numberi(match[count].rm_so);
        b->value.array->links[1] = create_numberi(match[count].rm_eo);
        array_push(a, b );
        count++;
    }

    return a;
}



static NATIVECALL(regex_matchAll){ 
    blue_regex br = This->value.vptr;
    regmatch_t match[64];
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    if ( argn != 1 ) return exception("ArgError", NULL,NULL);
    
    string_t s = object_getString( args[0]);
    
    if ( ! s) return exception("ArgError", NULL,NULL);    
    
    char * string = s->data;
    Link a = array_new(0);
    string_t mm;
    int length;
    int count = 0;
    int start=0;
    int end=0;
    
    while(  !  regexec(&(br->re),string,64,match,0)){
        //DPRINT("A  [%s]",string);
        count = 0;
        while ((count < 64)&&( match[count].rm_so != -1)){
            //DPRINT("   B  %i",count);
            start = match[count].rm_so;
            end = match[count].rm_eo;
            length = end - start;
            mm = string_new_ls(length, NULL);
            memcpy(mm->data, string + start, length);
            array_push(a, create_string_str( mm) );
            count++;
        }
        string+=end;
        
    }
    
    return a;
}






        //~ while( status = regexec( &re, ps, 1, pmatch, eflag)== 0){
                //~ printf("match found at: %d, string=%s\n", 
                        //~ pmatch[0].rm_so, ps +pmatch[0].rm_so);
                //~ ps += pmatch[0].rm_eo;
                //~ printf("\nNEXTString to match=%s\n", ps);
                //~ eflag = REG_NOTBOL;
 
        //~ }




//~ int main (int argc, char ** args){

    //~ char  string[] ="hello there how are you";
    //~ char pattern[] = "h.w";
    
    //~ regmatch_t match[64];
    
    //~ regex_t re ;
    
    //~ regcomp( &re, pattern, REG_EXTENDED);
    //~ regexec(&re,string,64,match,0); 
    //~ regfree(&re);
    
    //~ printf( " %i  %i\n", match[0].rm_so, match[0].rm_eo);
    
    
    //~ return 0;
//~ }


void init(INITFUNC_ARGS){
    
    regex_type = newNative();
        regex_type->create   = create;
        regex_type->destroy  = destroy;
        regex_type->asString = regex_asString;
        addNativeCall(regex_type, "print", universal_print);
        addNativeCall(regex_type, "match", regex_match);
        addNativeCall(regex_type, "matchAll", regex_matchAll);
        addNativeCall(regex_type, "exec",  regex_exec);
    
    
    addCFunc(Module, "pattern", regex_pattern);
}



