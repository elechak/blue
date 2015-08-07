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
#include "../stream.h"


EXPORT void init(INITFUNC_ARGS);


static NativeType stream_type       = NULL;
static NativeType streamserver_type = NULL;

/***************** Stream *****************/
static void  create(Link self){
    self->value.vptr =  NULL;
}

static void destroy(Link self){
    if (self->value.vptr)stream_close(self->value.vptr);
    self->value.vptr = NULL;
    object_destroy(self);
}

static number_t asNumber(Link self){
    return 0.0;
}

static string_t asString(Link self){
    return stream_read(self->value.vptr, 4096);
}

static int is_true(Link self){
    if (self->value.vptr) return 1;
    return 0;
}

/***************** Native class functions *****************/

static string_t file_stream_def_type;

static NATIVECALL(file_stream){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    string_t type = file_stream_def_type;
    string_t name = NULL; 
    mode_t mode = 0666;

    switch (argn){
        case 3:
            mode = object_asNumber( args[2]  );
        case 2:
            type = object_getString( args[1] );
        case 1:
            name = object_getString( args[0] );
            break;
        default:
            return exception("FileNameRequired", NULL, NULL);
    }

    Link link = object_create(stream_type);
    link->value.vptr = stream_open_file(name->data , type->data, mode);
    return link;
}

static NATIVECALL(socket_stream){
    Link * args = array_getArray(Arg);

    string_t location = object_getString( args[0] );

    Link link = object_create(stream_type);
    link->value.vptr = stream_open_socket(location->data);
    return link;
}

static NATIVECALL(exec_stream){
    Link * args = array_getArray(Arg);
    string_t command = object_getString( args[0] );
    Link link = object_create(stream_type);
    link->value.vptr = stream_open_exec(command->data);
    return link;
}

static NATIVECALL(mem_stream){
    Link link = object_create(stream_type);
    link->value.vptr = stream_open_mem();
    return link;
}

static NATIVECALL(shell_stream){
    Link link = object_create(stream_type);
    link->value.vptr = stream_open_shell();
    return link;
}

static NATIVECALL(stdio_stream){
    Link link = object_create(stream_type);
    link->value.vptr = stream_open_stdio();
    return link;
}


/***************** Native methodes *****************/

static NATIVECALL(read_stream){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    string_t ret = NULL;
    size_t amount = 0;
    Link obj = NULL;

    switch(argn){
        /* no arguments */
        case 0:
            ret = stream_read(This->value.vptr, 4096);
            break;

        case 1:
            obj = args[0];
            if (obj->type == Global->number_type){
                amount =(size_t) obj->value.number;
                ret = stream_read(This->value.vptr, amount);
            }else if (   object_getString( obj )  ){
                ret = stream_readbreak(This->value.vptr, object_getString(obj) );
            }else{
                return exception("InvalidTypeForArgument",NULL,NULL);
            }
            break;
    }

    if (! ret) return exception("StreamFinished", NULL, NULL);

    return create_string_str( ret );
}

static NATIVECALL(ready_stream){
    return create_numberi( stream_ready(This->value.vptr) );
}

static NATIVECALL(write_stream){
    Link * args = array_getArray(Arg);
    string_t content = object_getString(args[0]);
    stream_write(This->value.vptr , content);
    return link_dup(This);
}




/***************** StreamServer *****************/
static void  ss_create(Link self){
    self->value.vptr =  NULL;
}

static void ss_destroy(Link self){
    if (self->value.vptr)streamserver_close(self->value.vptr);
    self->value.vptr = NULL;
}

static number_t ss_asNumber(Link self){
    return 0.0;
}

static string_t ss_asString(Link self){
    return string_new_formatted("<Streamserver %p>",self->value.vptr);
}


static NATIVECALL(streamserver_create){
    Link * args = array_getArray(Arg);
    int port = object_asNumber(args[0]);

    StreamServer ss = stream_server(port);
    if (! ss) return exception("StreamListenError", NULL, NULL);

    Link link = object_create(streamserver_type);
    link->value.vptr = ss;
    return link;
}

static NATIVECALL(streamserver_accept){
    StreamServer ss = This->value.vptr;
    Link link = object_create(stream_type);
    link->value.vptr = stream_open_accept(ss);
    return link;
}


void init(INITFUNC_ARGS){
    
    file_stream_def_type = string_new("r");
    
    
    /* STREAM TYPE */
        stream_type = newNative();
            stream_type->lib            = lib;
            stream_type->create         = create;
            stream_type->destroy        = destroy;
            stream_type->asNumber       = asNumber;
            stream_type->asString       = asString;
            stream_type->is_true        = is_true;

        addNativeCall(stream_type, "read", read_stream);
        addNativeCall(stream_type, "write", write_stream);
        addNativeCall(stream_type, "ready", ready_stream);

    /* STREAMSERVER TYPE */
        streamserver_type = newNative();
            streamserver_type->lib            = lib;
            streamserver_type->create         = ss_create;
            streamserver_type->destroy        = ss_destroy;
            streamserver_type->asNumber       = ss_asNumber;
            streamserver_type->asString       = ss_asString;

        addNativeCall(streamserver_type, "accept", streamserver_accept);

    /* MODULE FUNCTIONS */
        addCFunc(Module, "file",   file_stream);
        addCFunc(Module, "exec",   exec_stream);
        addCFunc(Module, "shell",  shell_stream);
        addCFunc(Module, "memory", mem_stream);
        addCFunc(Module, "socket", socket_stream);
        addCFunc(Module, "stdio",  stdio_stream);
        addCFunc(Module, "listen", streamserver_create);
}


