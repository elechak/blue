/*
The blue programming language ("blue")
Copyright (C) 2007  Erik R Lechak

email: erik@leckak.info
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

EXPORT void init(INITFUNC_ARGS);
static NativeType xmlparser_type       = NULL;

struct XMLParser{
    char * tag;
    char * tags;
    char * data;
    Link onError;
    Link startElement;
    Link endElement;
    Link characters;
};
typedef struct XMLParser * XMLParser;


/*********************************/
static void  create(Link link){
    XMLParser self = link->value.vptr =  malloc(sizeof(*self));
    self->tag           = NULL;
    self->tags          = NULL;
    self->data          = NULL;
    self->onError       = NULL;
    self->startElement  = NULL;
    self->endElement    = NULL;
    self->characters    = NULL;
}

static void destroy(Link link){
    XMLParser self = link->value.vptr;
    if (! self) return;
        
    if (self->tag) free(self->tag);
    if (self->tags) free(self->tags);
    if (self->data) free(self->data);
        
    if (self->onError)         link_free(self->onError);
    if (self->startElement)    link_free(self->startElement);
    if (self->endElement)      link_free(self->endElement);
    if (self->characters)      link_free(self->characters);
        
    free(self);
    
    if (link->attribs) dictionary_free(link->attribs);
    free(link);
}


static int is_true(Link link){
    if (link->value.vptr) return 1;
    return 0;
}

static NATIVECALL(new_parser){
    return object_create(xmlparser_type);
};

static NATIVECALL(parsexml){
    Link * args = array_getArray(Arg);

    char * string = object_getString(args[0])->data;
    XMLParser self = This->value.vptr;

    Link ret = NULL;
    char *cursor = string;
    char *start  = NULL;
    char *end    = NULL;
    unsigned long length;

    self->tag  = malloc(512);
    self->tags = malloc(1024);

    self->tag[0] = 0;
    self->tags[0] = 0;

    while(1){
        if (! *cursor) break;

        /* find the start of the next tag */
        start = strchr(cursor, '<');
        if (! start ) goto error;

        /* if there are characters between the cursor and the next start tag call character handler */
        if (start != cursor){
            length = (unsigned long)start - (unsigned long)cursor ;
            self->data=realloc(self->data, length +1 );
            memcpy(self->data, cursor, length);
            self->data[length] = '\0';
            if (self->characters){
                ret = object_call(self->characters,  This,NULL);
                if (ret) link_free(ret);
            }
        }
        end = strchr(cursor, '>');

        if (! end) goto error;

        cursor = end+1; // move cursor pas the >

        // copy element data into self->data
        length = (unsigned long)end - (unsigned long)start ;
        self->data=realloc(self->data,length);
        memcpy(self->data , start+1 , length-1);
        self->data[length-1] = '\0';

        if (  *(self->data) == '?' ){

        }else if (  *(self->data) == '!' ){

        }else if (  *(self->data) == '/' ){
            length = strcspn(self->data, " >"); // stop reading tag at a space, >, or /

            /* copy tag into self->tag */
            strncpy(self->tag, self->data+1, length-1);
            self->tag[length-1] = 0;

            // END ELEMENT
            if (self->endElement){
                ret = object_call(self->endElement,  This,NULL);
                if (ret) link_free(ret);
            }
            start = strrchr(self->tags, '.'); // go up one level in self->tags
            *start = '\0';

        }else{
            // START ELEMENT
            length = strcspn(self->data, " />"); // stop reading tag at a space, >, or /

            /* copy tag into self->tag */
            strncpy(self->tag, self->data, length);
            self->tag[length] = 0;

            /* append tag into self->tags */
            strcat(self->tags, ".");
            strcat(self->tags,self->tag);

            if (self->startElement){
                ret = object_call(self->startElement,  This,NULL);
                if (ret) link_free(ret);
            }

            // check to see if the is a single tag element
            if ( self->data[length] == '/'){
                if (self->endElement){
                    ret = object_call(self->endElement,  This,NULL);
                    if (ret) link_free(ret);
                }
                start = strrchr(self->tags, '.');
                *start = '\0';
            }
        }
    }
    
    if (0){
        error:
            if (self->onError){
                    link_free( object_call(self->onError,  This,NULL));
            }
    }
    
    return object_create(Global->null_type);
}


static NATIVECALL(onError){
    Link * args = array_getArray(Arg);
    XMLParser self = This->value.vptr;
    self->onError = link_dup(args[0]);
    return object_create(Global->null_type);
}

static NATIVECALL(startElement){
    Link * args = array_getArray(Arg);
    XMLParser self = This->value.vptr;
    self->startElement = link_dup(args[0]);
    return object_create(Global->null_type);
}

static NATIVECALL(endElement){
    Link * args = array_getArray(Arg);
    XMLParser self = This->value.vptr;
    self->endElement = link_dup(args[0]);
    return object_create(Global->null_type);
}

static NATIVECALL(characters){
    Link * args = array_getArray(Arg);
    XMLParser self = This->value.vptr;
    self->characters = link_dup(args[0]);
    return object_create(Global->null_type);
}

static NATIVECALL(getTag){
    XMLParser self = This->value.vptr;
    return create_string(self->tag);
}

static NATIVECALL(getTags){
    XMLParser self = This->value.vptr;
    return create_string(self->tags);
}

static NATIVECALL(getData){
    XMLParser self = This->value.vptr;
    return create_string(self->data);
}


void init(INITFUNC_ARGS){
    xmlparser_type = newNative();
        xmlparser_type->lib            = lib;
        xmlparser_type->create         = create;
        xmlparser_type->destroy        = destroy;
        xmlparser_type->is_true        = is_true;

    addNativeCall(xmlparser_type, "parse", parsexml);
    addNativeCall(xmlparser_type, "onError", onError);
    addNativeCall(xmlparser_type, "startElement", startElement);
    addNativeCall(xmlparser_type, "endElement", endElement);
    addNativeCall(xmlparser_type, "characters", characters);

    addNativeCall(xmlparser_type, "getTag", getTag);
    addNativeCall(xmlparser_type, "getTags", getTags);
    addNativeCall(xmlparser_type, "getData", getData);

    addCFunc(Module, "parser", new_parser);
}


