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
#include "bstring.h"

string_t string_new(const char * cstring){
    size_t length = strlen(cstring);
    return string_new_ls(length , cstring);
}

string_t string_new_ls(size_t length, const char * cstring){
    string_t self = malloc(sizeof(*self)+length+1);
    if ( ! self){
        printf("string_new_ls memory error (malloc size=%lu)\n",sizeof(*self)+length+1 );
        exit(1);
    }
    self->length = length;
    if (cstring) memcpy(self->data, cstring, length);
    self->data[length]=0;
    return self;
}

string_t string_new_add(string_t a, string_t b){
    if ( (!a)  && (!b) ) return NULL;

    if ( !a )    return string_dup(b);
    else if (!b) return string_dup(a);

    string_t self = string_new_ls(a->length + b->length, NULL);
    memcpy(self->data, a->data, a->length);
    memcpy(self->data + a->length, b->data, b->length);
    self->data[self->length]=0;
    return self;
}

string_t string_new_merge(string_t a, string_t b, int keep_a, int keep_b){
    if ( (!a)  && (!b) ) return NULL;

    if ( !a )    return string_dup(b);
    else if (!b) return string_dup(a);

    string_t self = string_new_ls(a->length + b->length, NULL);
    memcpy(self->data, a->data, a->length);
    memcpy(self->data + a->length, b->data, b->length);
    self->data[self->length]=0;
    if (! keep_a)  free(a);
    if (! keep_b)  free(b);
    return self;
}

string_t string_new_formatted(const char * format, ...){
    string_t self = NULL;
    
    /* find length */
    int length = -1;
    char * buffer = NULL;
    int buffer_size = 0;
    va_list va;
    
    va_start(va, format);
    length = vsnprintf(NULL,0,format,va);
    va_end(va);
    
    while( length < 0 ){
        buffer_size +=1000;
        buffer = malloc( buffer_size );
        va_start(va, format);
        length = vsnprintf(buffer,1000,format,va);
        va_end(va);
        free(buffer);
    }    
    
    self = string_new_ls(length,NULL);
    va_start(va, format);
    length = vsnprintf(self->data,self->length+1,format,va);
    va_end(va);
    return self;
}

string_t string_new_quoted(const char * cstring, char ** pcstring){
    const char start = *cstring;
    char stop;
    string_t self = NULL;
    size_t i = 0;
    size_t length = 0;
    const char * pchar = cstring;
    char c;

    switch (start){
        case '"':
            stop = '"';
            break;
        case '\'':
            stop = '\'';
            break;
        default:
            stop = '\0';
    }

    pchar++; // go one past the start character

    // count the length
    while( (c=*pchar) && (*pchar != stop)){
        if (c == '\\'){
            pchar++;
        }
        length++;
        pchar++;
    }

    // create a string of the desired length
    self = string_new_ls(length,NULL);

    pchar = cstring+1;// one past start character

    // copy into new string
    while( (c=*pchar) && (*pchar != stop)){
        if (c == '\\'){
            pchar++;
            c = *pchar;
            switch(c){
                case 'n':
                    c = '\n';
                    break;
            }
        }
        self->data[i++] = c;
        pchar++;
    }

    *pcstring = (char *) ++pchar;
    return self;
}


string_t string_toUpper( const string_t s){
    string_t a = string_new_ls( s->length, NULL);
    
    char * dest = a->data;    
    char * source = s->data;
    
    while( *source) *dest++ = ((*source > 96) &&(*source<123))   ?  *source++ -32  : *source++;
    return a;
}

string_t string_toLower( const string_t s){
    string_t a = string_new_ls( s->length, NULL);
    
    char * dest = a->data;    
    char * source = s->data;
    
    while( *source) *dest++ = ((*source > 64) &&(*source<91))   ?  *source++ +32  : *source++;
    
    return a;
}


string_t string_substr(const string_t s, size_t start, size_t end){
    return string_new_ls(end-start, s->data+start);
}

string_t string_dup(const string_t s){
    if ( ! s) return NULL;
    return string_new_ls(s->length, s->data);
}

size_t string_sizeof(const string_t self){
    if (! self) return 0;
    return (sizeof(struct string_header) + self->length +1);
}

size_t string_length(const string_t self){
    if (! self) return 0;
    return (self->length);
}

int string_compare(string_t a, string_t b){
    if (a->data == b->data) return 0;
    
    int length = a->length;
    
    if ( b->length < length) length = b->length;
    
    int cmp = memcmp(a->data ,b->data,length);
    
    if (cmp ==0){
        if (a->length > b->length) return 1;
        else if (a->length < b->length) return -1;
    }
    return cmp;
}

int string_compare_cstring(string_t a, char * cstring){
    if (a->data == cstring) return 0;
        
    int cstring_length = strlen(cstring);
    
    int length = a->length;
    
    if ( cstring_length < length) length = cstring_length;
    
    int cmp = memcmp(a->data ,cstring,length);
    
    if (cmp ==0){
        if (a->length > cstring_length) return -1;
        else if (a->length < cstring_length) return 1;
    }    
    
    return cmp;
}


void string_fprint(const string_t self ,FILE * stream){
    int c;
    size_t length;
    length = self->length;
    for(c = 0; c < length ; ++c){
        fputc(self->data[c] , stream);
    }
}

size_t string_find(string_t big, string_t little){
    if (
        (!big)||
        (!little)||
        (!big->length)||
        (!little->length)||
        (little->length > big->length)
    ) return 0;

    size_t end = big->length - little->length+1;
    size_t c;
    for (c=0 ; c < end; c++){
        if ((big->data[c] == little->data[0]) && (memcmp(big->data+c, little->data, little->length) ==0 )){
           return c+little->length;
        }
    }
    return 0;
};

size_t string_save( string_t self, char * filename){
    FILE * fp = fopen(filename, "wb");
    if (! fp ) return 0;
    size_t n = fwrite(self->data,1,self->length,fp);
    fclose(fp);
    return n;
}

string_t string_load( const char * filename){
    FILE * fp =  fopen(filename , "rb");
    if (! fp) return NULL;
    fseek(fp, 0 , SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0 , SEEK_SET);
    
    string_t self = string_new_ls(file_size,NULL);
    
    fread(self->data , file_size, 1,fp);
    fclose(fp);    
    return self;
}

int string_endsWith( string_t self, string_t test){
    if (test && (test->length <= self->length)  ){
        if (memcmp(test->data, self->data + self->length - test->length, test->length)==0) {
            return 1;
        }
    }    
    return 0;
}

int string_startsWith( string_t self, string_t test){
    if (test && (test->length <= self->length)  ){
        if (memcmp(test->data, self->data, test->length)==0) {
            return 1;
        }
    }  
    return 0;
}


string_t string_ltrim( string_t self, string_t trash){
    int length = strspn( self->data , trash->data);
    return string_substr(self, length, self->length);    
}



string_t string_rtrim( string_t self, string_t trash){
    int c;
    int i;
    int trash_length = trash->length;
    
    i = self->length -1;
    while( i != -1){
        for ( c=0; c < trash_length; c++){
            if (self->data[i] == trash->data[c]) goto found;
        }
        break;
        found:
        i--;
    }
    return string_substr(self, 0, i+1);    
}


string_t string_trim(string_t self, string_t trash){
    int start = strspn( self->data , trash->data);

    int end;
    int c;
    int trash_length = trash->length;
    
    end = self->length -1;
    while( end != -1){    
        for ( c=0; c < trash_length; c++){
            if (self->data[end] == trash->data[c]) goto found;
        }
        break;
        found:
        end--;
    }
    return string_substr(self, start, end+1);    
}


string_t string_replace( string_t self, string_t pattern , string_t b){
    
    char * pos = NULL;

    int start_index = 0;
    int end_index = 0;
    
    string_t temp = string_new("");
    
    while( (pos = strstr( &(self->data[start_index]), pattern->data))){
        end_index = pos - self->data;    
        temp = string_new_merge( temp , string_substr(self,start_index,end_index) ,0 , 0 );
        temp = string_new_merge( temp , b , 0 , 1 );
        start_index = end_index + pattern->length;
    }
    
    end_index = self->length;
    temp = string_new_merge( temp , string_substr(self,start_index,end_index) ,0 , 0 );
    return temp;
}



//~ int string_scprintf(const char * format, ...){
    //~ int length = -1;
    //~ char * buffer = NULL;
    //~ int buffer_size = 0;
    //~ va_list va;
    
    //~ va_start(va, format);
    //~ length = vsnprintf(NULL,0,format,va);
    //~ va_end(va);
    
    //~ while( length < 0 ){
        //~ buffer_size +=1000;
        //~ buffer = malloc( buffer_size );
        //~ va_start(va, format);
        //~ length = vsnprintf(buffer,1000,format,va);
        //~ va_end(va);
        //~ free(buffer);
    //~ }

    //~ return length;
//~ }












