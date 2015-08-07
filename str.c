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

#include "str.h"


static NativeType string_type;
static NativeType string_literal_type;

Link create_string(const char * string){
    Link link  = object_create(string_type);
    link->value.string = string_new(string ? string : "");
    return link;
}

Link create_string_literal(void * string){
    Link link  = object_create(string_literal_type);
    link->value.string = string;
    return link;
}

Link create_string_str(string_t string){
    Link link  = object_create(string_type);
    link->value.string = string;
    return link;
}


static void destroy(Link self){
    if (self->value.string) free(self->value.string);
    object_destroy(self);  
}

static void destroy_literal(Link self){
    /* dont free the string for string literals */
    object_destroy(self);  
}

/* return the internal string, only string type objects should implement this vfunction */
static string_t getString(Link self){
    return self->value.string;
}

static Link copy(Link self){    
    return create_string_str( string_dup( object_getString(self)) );
}

static number_t asNumber(Link self){
    return (number_t) strtod( object_getString(self)->data , NULL);
}

static string_t asString(Link self){
    return string_dup(self->value.string);
}

static Link op_plus(Link self, Link other){
    
    string_t other_string = object_getString( other );
    
    if ( other_string ){
        return create_string_str( string_new_add(self->value.string, other_string) );
    }else{
        other_string = object_asString( other );
        Link obj = create_string_str(string_new_add(self->value.string, other_string));
        free(other_string);
        return obj;
    }
}

static int is_true(Link self){
    string_t s = self->value.string;
    if (! s) return 0;
    if (s->length) return 1;
    return 0;
}



static int compare(Link self, Link other){
    string_t a = self->value.string;
    string_t b = object_getString( other );
    
    if (a == b) return 0;
        
    int result = 0;
    
    if ( b ){
        result = string_compare(a, b);
    }
    
    return result;
}


/***************** Native methods *****************/
static NATIVECALL(str_length){
    return create_numberi((number_t)  This->value.string->length) ;
}

/* find the position of 'test' in 'string' advanced by offset */
static int _str_find(string_t string, string_t test, size_t offset){
    char * pchar = NULL;
    if (!(test && (test->length <= string->length-offset))){
        return -1;
    }
    pchar = strstr(string->data+offset, test->data);
    if (! pchar) return -1;
    return (pchar - string->data);
}

/* find the position of args[0] in This */
static NATIVECALL(str_find){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    size_t offset = 0;
    int index = -1;
    switch(argn){
        case 2:
            offset = object_asNumber(args[1]);
        case 1:
            index = _str_find(This->value.string, object_getString( args[0] ) , offset);  
    }

    if (index == -1 ){
        return exception("StringFindFailed", NULL, NULL);
    }

    return create_numberi(index);
}

static NATIVECALL(str_substr){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    Link arg0 = NULL;
    Link arg1 = NULL;

    /* main string */
    string_t string = This->value.string; 

    int start = 0;
    int end   = string->length;

    switch (argn){
        case 2:
            arg1 = args[1];
        case 1:
            arg0 = args[0];
            break;
    }

    if (arg0){
        string_t start_string = object_getString( arg0 );
        if (start_string){
            start = _str_find(string, start_string,0);
            if (start == -1 ) return exception("StartIndexError", NULL, NULL);
        }else{
            start = object_asNumber(arg0);
        }
    }

    if (arg1){
        string_t end_string = object_getString( arg1 );
        if ( end_string ){
            end = _str_find(string, end_string, start+1);
            if (end == -1 ) return exception("EndIndexError", NULL, NULL);
        }else{
            end = object_asNumber(arg1);
        }
    }

    if (start < 0){
        start = string->length + start;
        if (start <0){
            return exception("StrSubstrStartIndexInvalid", NULL, NULL);
        }
    }

    if (end < 0){
        end = string->length + end;
        if (start <0){
            return exception("StrSubstrEndIndexInvalid", NULL, NULL);
        }
    }

    //printf("%i %i\n", start, end);
    if (start > end){
        return exception("StrSubstrEndGreaterThanStart", NULL, NULL);
    }

    return create_string_str( string_substr(string, start, end) );
}

static NATIVECALL(str_startsWith){
    Link * args = array_getArray(Arg);
    string_t string = This->value.string;
    string_t test = object_getString( args[0] );
    return create_numberi( string_startsWith( string , test));
}

static NATIVECALL(str_endsWith){
    Link * args = array_getArray(Arg);
    string_t string = This->value.string;
    string_t test = object_getString( args[0] );
    return create_numberi( string_endsWith( string , test));
}

//~ static NATIVECALL(str_print){    
    //~ Link * args = array_getArray(Arg);
    //~ size_t argn  = array_getLength(Arg);

    //~ fwrite( This->value.string->data, 1, This->value.string->length, stdout);
    
    //~ size_t count;
    //~ string_t s = NULL;

    //~ for ( count = 0; count <argn; count++){
        //~ s = object_asString(args[count]);
        //~ fwrite(s->data, 1,s->length,stdout);
        //~ free(s);
    //~ }
    //~ fflush(stdout);
    //~ return object_create(Global->null_type);
//~ };

static NATIVECALL(str_num){    
    return create_number(asNumber(This));
};

static NATIVECALL(str_split){
    int start = 0;
    int end;

    Link * args = array_getArray(Arg);

    string_t string = This->value.string;
    string_t test = object_getString( args[0] );
    
    Link ret = array_new(0);
    
    /* if test is not a string convert it to a number and use it as an index */
    if (! test ){
        int index = object_asNumber( args[0] );
        if (index > string->length) index = string->length;
        array_push(ret, create_string_str( string_substr(string, 0,index) ));
        array_push(ret, create_string_str( string_substr(string, index,string->length) ));
        return ret;
    }
        
    while(1){
        end = _str_find(string,test, start);
        if (end == -1){ 
            array_push(ret,  create_string_str( string_substr(string, start,string->length) ) );
            return ret;
        }
        array_push(ret, create_string_str( string_substr(string, start,end) )  );
        start = end + test->length;
    }
    
    return ret;
}


static NATIVECALL(loadFromFile){

    string_t filename = object_getString( This );
    
    string_t s = NULL;
    
    if ( filename ){
        s = string_load(filename->data);
    }
        
    if (! s ) return exception("CouldNotLoadString", NULL, NULL);

    return create_string_str( s );
}



static NATIVECALL(saveToFile){
    Link * args = array_getArray(Arg);
    
    string_t filename = object_getString( args[0] );

    FILE * fp = NULL;
    
    if ( filename ){
        fp = fopen(filename->data, "wb");
    }

    if (! fp){
        return exception("CouldNotOpenFile", NULL, NULL);
    }

    string_t str = This->value.string;

    fwrite(str->data, string_length(str),1,fp);
    fclose(fp);

    return link_dup(This);
}

static NATIVECALL(str_repeat){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if ((argn != 1) || ( args[0]->type != Global->number_type) ){
        return exception("ArgumentError", NULL, NULL);
    }
    
    string_t original  = This->value.string;
    
    int repeat = object_asNumber(args[0]);
    
    string_t newstring = string_new_ls(original->length * repeat , NULL );
    
    int c;
    char * target = newstring->data;
    for (c =0; c<repeat; c++, target+=original->length){
        memcpy(target, original->data,original->length);
    }

    return create_string_str( newstring );
}


static NATIVECALL(str_Compile){
    Link module = create_module_string(This->value.string);
    
    /* See if this is an error */
        if ( is_critical(module) ) return module;
    
    /* create a function object */
    Link cbo = object_create(Global->function_type);
    CodeBlock cb = cbo->value.codeblock;
    cb->parent   = module;
    cb->bytecode = module->value.module->bytecode;
    return cbo;
}


static NATIVECALL(str_eval){
    Link module = create_module_string(This->value.string);
    
    /* See if this is an error */
        if ( is_critical(module) ) return module;
    
    /* create a function object */
    Link cbo = object_create(Global->function_type);
    CodeBlock cb = cbo->value.codeblock;
    cb->parent   = module;
    cb->bytecode = module->value.module->bytecode;

    Link ret = object_call(cbo, link_dup(module),Arg);

    link_free(cbo);
    
    if (ret){
        link_free(module);
        return ret;
    }
    
    return module;  
}




string_t trash_default = NULL;

static NATIVECALL(str_ltrim){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    string_t trash = NULL;
    if (argn){ 
        trash = object_getString( args[0]);
        
        if (! trash) if (args[0]->type == Global->number_type){
            return create_string_str( string_substr( This->value.string, (int)(args[0]->value.number), This->value.string->length ));
        }
    }

    if (! trash) trash = trash_default;
    
    return create_string_str( string_ltrim( This->value.string, trash) );
}

static NATIVECALL(str_rtrim){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    string_t trash = NULL;
    if (argn){ 
        trash = object_getString( args[0]);
        
        if (! trash) if (args[0]->type == Global->number_type){
            return create_string_str( string_substr( This->value.string, 0, This->value.string->length - (int)(args[0]->value.number) ));
        }
    }

    if (! trash) trash = trash_default;
    
    return create_string_str( string_rtrim( This->value.string, trash) );
}



static NATIVECALL(str_trim){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    string_t trash = NULL;
    string_t trash_start = NULL;
    string_t trash_end   = NULL;
    string_t ltrimmed = NULL;
    string_t rtrimmed = NULL;    
    
    switch( argn ){
        case 0:
            return create_string_str( string_trim( This->value.string, trash_default) );
        case 1:
            
            trash = object_getString( args[0]);
            if (! trash) {
                if (args[0]->type == Global->number_type){
                    return create_string_str( string_substr( This->value.string, (int)(args[0]->value.number), This->value.string->length - (int)(args[0]->value.number) ));
                }
                
                trash = trash_default;
            }

            return create_string_str( string_trim( This->value.string, trash) );
            
        default:
            trash_start = object_getString(args[0]);
            trash_end   = object_getString(args[1]);
        
            if ( ! trash_start ){
                if (args[0]->type == Global->number_type){
                    int index_start = (int)(args[0]->value.number);
                    ltrimmed = string_substr( This->value.string, index_start, This->value.string->length);
                }else{
                    ltrimmed = string_ltrim( This->value.string, trash_default);
                }
            }else{
                   ltrimmed = string_ltrim( This->value.string, trash_start);
            }
        
            if ( ! trash_end ){
                if (args[1]->type == Global->number_type){
                    int index_end = (int)(args[1]->value.number);
                    rtrimmed = string_substr( ltrimmed, 0, ltrimmed->length - index_end);
                }else{
                    rtrimmed = string_rtrim( ltrimmed, trash_default);
                }
            }else{
                   rtrimmed = string_rtrim( ltrimmed, trash_end);
            }
            
            free(ltrimmed);
            
            return create_string_str( rtrimmed);
    }
    return NULL;
}


static NATIVECALL(str_replace){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    if (argn < 2) return exception("ArgumentError", NULL, NULL);
    
    string_t s = object_getString( This );
    if (! s) return exception("CallerError", NULL, NULL);
    
    return create_string_str( string_replace( s , object_getString(args[0]), object_getString(args[1]) ) );
}


static NATIVECALL(str_import){
    return module_import( This, Arg);
}



NativeType str_init(){

    trash_default = string_new( " \n\r\t");
    
    string_type = newNative();
        string_type->destroy        = destroy;
        string_type->copy           = copy;
        string_type->asNumber       = asNumber;
        string_type->asString       = asString;
        string_type->compare        = compare;
        string_type->op_plus        = op_plus;
        string_type->is_true        = is_true;
        string_type->getString      = getString;

        addNativeCall(string_type, "import", str_import);
        addNativeCall(string_type, "compile", str_Compile);
        addNativeCall(string_type, "eval", str_eval);

        addNativeCall(string_type, "load", loadFromFile);
        addNativeCall(string_type, "save", saveToFile);
    
        addNativeCall(string_type, "print", universal_print);
        addNativeCall(string_type, "num", str_num);
        addNativeCall(string_type, "repeat", str_repeat);
        addNativeCall(string_type, "substr", str_substr);
        addNativeCall(string_type, "find", str_find);
        addNativeCall(string_type, "split", str_split);
        addNativeCall(string_type, "startsWith", str_startsWith);
        addNativeCall(string_type, "endsWith", str_endsWith);
        addNativeCall(string_type, "length", str_length);
        addNativeCall(string_type, "ltrim", str_ltrim);
        addNativeCall(string_type, "rtrim", str_rtrim);
        addNativeCall(string_type, "trim", str_trim);
        addNativeCall(string_type, "replace", str_replace);
    
    string_literal_type = newNative( string_type );
        string_literal_type->destroy        = destroy_literal;
        string_literal_type->copy           = copy;
        string_literal_type->asNumber       = asNumber;
        string_literal_type->asString       = asString;
        string_literal_type->compare        = compare;
        string_literal_type->op_plus        = op_plus;
        string_literal_type->is_true        = is_true;
        string_literal_type->getString      = getString;
        
        static Link string_literal_type_link;
        string_literal_type_link= string_literal_type->type_link;
        string_literal_type->type_link = string_type->type_link;      
    
    return string_type;
}


