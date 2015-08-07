/*
The blue programming language ("blue")
Copyright (C) 2007-2008  Erik R Lechak

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

#include "graphics.h"

Index bound_layers;

void layer_transform(Layer self){
    Event event = &(current_window->event);
    if (self->transform){
        glmat_multInplace(event->transform , self->transform);
    }
}


void layer_draw(Link link){

    Layer self =  link->value.vptr;

    if ( (! self->children) || (! self->children->length)) return;

    // remember some important states
    unsigned long draw_status_save = draw_status;

    glPushAttrib(  GL_ALL_ATTRIB_BITS );

    //This is the other transforms
    if (self->transform) glMultMatrixd(self->transform);

    // only wory about name stack during a select
    if (draw_status & SELECT){
        
        if (self->bound_id){
            glPushName( (GLuint) self->bound_id); // name for hit testing
        }
        
    }else if (self->bias){
            glPixelTransferf( GL_RED_BIAS,     self->bias[0]);
            glPixelTransferf( GL_GREEN_BIAS,   self->bias[1]);
            glPixelTransferf( GL_BLUE_BIAS,    self->bias[2]);
            glPixelTransferf( GL_ALPHA_BIAS,   self->bias[3]);
    }

    // Draw Children
    drawChildren((ChildMan)self);

    // return saved states
    glPopAttrib(); // make sure material, lighting, and texture don't propagate up
    glGetDoublev(GL_CURRENT_RASTER_POSITION, raster_pos);
    draw_status = draw_status_save;

    if (draw_status & SELECT) glPopName();
}



static NATIVECALL(layer_new){
    Layer self = malloc(sizeof(*self));

    self->children  = list_new(sizeof(ListNode));

    self->transform    = NULL;
    self->position     = NULL;
    self->bindings     = NULL;
    self->bias         = NULL;
    
    Link obj = object_create(layer_type);
    obj->value.vptr = self;

    self->bound_id     = index_add(bound_layers, obj);
    
    return obj;
}

static void destroy(Link link){
    Layer self  = link->value.vptr;
    if (self->transform) free(self->transform);
    if (self->position) free(self->position);

    if (self->bound_id){
        index_remove(bound_layers , self->bound_id);
    }
    
    Binding binding = self->bindings;
    Binding temp = NULL;
    while(binding){
        link_free(binding->callback);
        temp = binding->next;
        free(binding);
        binding = temp;
    }

    freeChildren((ChildMan)self);
    free(self);
}


static NATIVECALL(layer_move){
    Layer self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    if (! self->position){
        self->position = malloc(sizeof(*(self->position)) * 3);
        self->position[0] = 0;
        self->position[1] = 0;
        self->position[2] = 0;
    }
    self->position[0] += object_asNumber(args[0]);
    self->position[1] += object_asNumber(args[1]);
    self->position[2] += object_asNumber(args[2]);

    return link_dup(This);
}

static NATIVECALL(layer_translate){
    Layer self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    if (! self->transform){
        self->transform = malloc(sizeof(*(self->transform)) * 16);
        glmat_loadIdent(self->transform);
    }

    
    glmat_translate(self->transform, object_asNumber(args[0]), object_asNumber(args[1]),- object_asNumber(args[2]));

    return link_dup(This);
}


static NATIVECALL(layer_rotate){
    Layer self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    if (! self->transform){
        self->transform = malloc(sizeof(*(self->transform)) * 16);
        memcpy(self->transform, identity_mat, sizeof(identity_mat) );
    }

    glmat_rotate(self->transform, object_asNumber(args[0]), object_asNumber(args[1]),object_asNumber(args[2]),object_asNumber(args[3]));

    return link_dup(This);
}

static NATIVECALL(layer_scale){
    Layer self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    if (! self->transform){
        self->transform = malloc(sizeof(*(self->transform)) * 16);
        memcpy(self->transform, identity_mat, sizeof(identity_mat) );
    }

    glmat_scale(self->transform, object_asNumber(args[0]), object_asNumber(args[1]),object_asNumber(args[2]));

    return link_dup(This);
}


static NATIVECALL(layer_bias){
    Layer self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    if (! self->bias){
        self->bias = malloc(sizeof(float) * 4);
        self->bias[0] = 0;
        self->bias[1] = 0;
        self->bias[2] = 0;
        self->bias[3] = 0;
    }
    self->bias[0] = object_asNumber(args[0]);
    self->bias[1] = object_asNumber(args[1]);
    self->bias[2] = object_asNumber(args[2]);
    self->bias[3] = object_asNumber(args[3]);

    return link_dup(This);
}



static char * binding_tokenize(char * string, char * token, const char * garbage){
    register size_t length;
    string += strspn(string,garbage);// skip the garbage
    length = strcspn(string,garbage);//read til garbage

    if (! length ) return NULL;

    for (; length>0; length--) *token++ = *string++;

    *token = '\0';
    return string;
}


Binding binding_new(char * string, Link callback){
    Binding self = malloc(sizeof(*self));
    self->next = NULL;
    self->callback = callback;
    Event evt = &(self->event);

    char token[16];
    char key[16];
    evt->state = 0;
    evt->key = 0;

    while ((string = binding_tokenize(string, token,BEVT_DELIM) )  ){
        if      ( strcmp(token, "shift"     )==0 ) evt->state |= BEVT_SHIFT;
        else if ( strcmp(token, "shiftlock" )==0 ) evt->state |= BEVT_SHIFTLOCK;
        else if ( strcmp(token, "ctrl"      )==0 ) evt->state |= BEVT_CTRL;
        else if ( strcmp(token, "alt"       )==0 ) evt->state |= BEVT_ALT;
        else if ( strcmp(token, "numlock"   )==0 ) evt->state |= BEVT_NUMLOCK;
        else if ( strcmp(token, "win"       )==0 ) evt->state |= BEVT_WIN;
        else if ( strcmp(token, "key"       )==0 ) evt->state |= BEVT_KEY;
        else if ( strcmp(token, "mouse"     )==0 ) evt->state |= BEVT_MOUSE;
        else if ( strcmp(token, "press"     )==0 ) evt->state |= BEVT_PRESS;
        else if ( strcmp(token, "release"   )==0 ) evt->state |= BEVT_RELEASE;
        else if ( strcmp(token, "motion"    )==0 ) evt->state |= BEVT_MOUSE | BEVT_MOTION;
        else if ( strcmp(token, "close"     )==0 ) evt->state |= BEVT_CLOSE;
        else if ( strcmp(token, "1"         )==0 ) evt->state |= BEVT_MOUSE | BEVT_MOUSE1;
        else if ( strcmp(token, "2"         )==0 ) evt->state |= BEVT_MOUSE | BEVT_MOUSE2;
        else if ( strcmp(token, "3"         )==0 ) evt->state |= BEVT_MOUSE | BEVT_MOUSE3;
        else if ( strcmp(token, "4"         )==0 ) evt->state |= BEVT_MOUSE | BEVT_MOUSE4;
        else if ( strcmp(token, "5"         )==0 ) evt->state |= BEVT_MOUSE | BEVT_MOUSE5;

        else if ( *token == '\'' ){
            binding_tokenize(token,key,"' ");
            if (strlen(key) == 1){
                evt->key = (long) *key;
                evt->state |= BEVT_KEY;
            }else{
                fprintf(stderr,"UNKNOWN KEY %s\n", key);
            }
        }else if ( *token == '#' ){
            evt->key = strtol(token+1,NULL,10);
        }
    }

    return self;
}

static NATIVECALL(layer_eventDescription){
    
    string_t str = string_new("");
    
    if ( current_window->event.state & BEVT_SHIFT )     str = string_new_merge(str, string_new("shift "),0,0);
    if ( current_window->event.state & BEVT_SHIFTLOCK ) str = string_new_merge(str, string_new("shiftlock "),0,0);
    if ( current_window->event.state & BEVT_CTRL )      str = string_new_merge(str, string_new("ctrl "),0,0);
    if ( current_window->event.state & BEVT_ALT )       str = string_new_merge(str, string_new("alt "),0,0);
    if ( current_window->event.state & BEVT_NUMLOCK )   str = string_new_merge(str, string_new("numlock "),0,0);
    if ( current_window->event.state & BEVT_WIN )       str = string_new_merge(str, string_new("win "),0,0);
    if ( current_window->event.state & BEVT_KEY )       str = string_new_merge(str, string_new("key "),0,0);
    if ( current_window->event.state & BEVT_MOUSE )     str = string_new_merge(str, string_new("mouse "),0,0);
    if ( current_window->event.state & BEVT_PRESS )     str = string_new_merge(str, string_new("press "),0,0);
    if ( current_window->event.state & BEVT_RELEASE )   str = string_new_merge(str, string_new("release "),0,0);
    if ( current_window->event.state & BEVT_MOTION )    str = string_new_merge(str, string_new("motion "),0,0);
    if ( current_window->event.state & BEVT_MOUSE1 )    str = string_new_merge(str, string_new("mouse1 "),0,0);
    if ( current_window->event.state & BEVT_MOUSE2 )    str = string_new_merge(str, string_new("mouse2 "),0,0);
    if ( current_window->event.state & BEVT_MOUSE3 )    str = string_new_merge(str, string_new("mouse3 "),0,0);
    if ( current_window->event.state & BEVT_MOUSE4 )    str = string_new_merge(str, string_new("mouse4 "),0,0);
    if ( current_window->event.state & BEVT_MOUSE5 )    str = string_new_merge(str, string_new("mouse5 "),0,0);
        
    if ( current_window->event.key ){
        str = string_new_merge(str, string_new_formatted("#%i" , current_window->event.key), 0,0);
    }
    
    return create_string_str(str);
}


static NATIVECALL(layer_bind){
    Layer self   = This->value.vptr;
        
    Link * args  = array_getArray(Arg);

    string_t string_event = object_getString(args[0]);
    Binding binding = binding_new(string_event->data, link_dup(args[1]));

    binding->next = self->bindings;
    self->bindings = binding;

    return link_dup(This);
}

static NATIVECALL(layer_eventWinPosition){
    Link pos_array = array_new(3);
    array_set(pos_array, 0, create_numberi(  pick_x ) );
    array_set(pos_array, 1, create_numberi(  pick_y ) );
    array_set(pos_array, 2, create_numberi(  0 ) );    
    return pos_array;
}

static NATIVECALL(layer_eventDWinPosition){
    Link pos_array = array_new(3);
    array_set(pos_array, 0, create_numberi(  pick_dx ) );
    array_set(pos_array, 1, create_numberi(  pick_dy ) );
    array_set(pos_array, 2, create_numberi(  0 ) );        
    return pos_array;
}


static NATIVECALL(layer_eventPos){
    Link pos_array = array_new(3);
    
    GLdouble x = pick_x;
    GLdouble y = pick_y;
    GLdouble z = 0;
    
    GLdouble inv_trans[16];
    
    glmat_inv(current_window->event.transform, inv_trans);
    
    glmat_apply3(inv_trans , &x, &y, &z);
    
    array_set(pos_array, 0, create_numberd( x ) );
    array_set(pos_array, 1, create_numberd(  y ) );
    array_set(pos_array, 2, create_numberd(  z ) );     
        
    return pos_array;
}


static NATIVECALL(layer_eventLayerPos){
    //Layer self   = This->value.vptr;
    
    Link pos_array = array_new(3);

    array_set(pos_array, 0, create_numberi(  pick_x ) );
    array_set(pos_array, 1, create_numberi(  pick_y ) );
    array_set(pos_array, 2, create_numberi(  0 ) );  
    return pos_array;
}


static NATIVECALL(layer_transform_apply){
    Layer self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    
    Link * array = array_getArray(args[0]);
    
    GLdouble x = object_asNumber(array[0]);
    GLdouble y = object_asNumber(array[1]);
    GLdouble z = object_asNumber(array[2]);
    
    GLdouble mat[16];
    if (self->transform)glmat_inv(self->transform , mat);    
    
    if (self->transform) glmat_apply3(mat , &x, &y, &z);
    
    Link pos_array = array_new(3);
    array_set(pos_array, 0, create_numberd( x ) );
    array_set(pos_array, 1, create_numberd(  y ) );
    array_set(pos_array, 2, create_numberd(  z ) );     
    
    return pos_array;
}


static NATIVECALL(layer_eventPosition){
    Link pos_array = array_new(3);
    array_set(pos_array, 0, create_numberd( current_window->event.pos.x ) );
    array_set(pos_array, 1, create_numberd(  current_window->event.pos.y ) );
    array_set(pos_array, 2, create_numberd(  current_window->event.pos.z ) );      
    return pos_array;
}

static NATIVECALL(layer_eventDPosition){
    Link pos_array = array_new(3);
    array_set(pos_array, 0, create_numberd( current_window->event.dpos.x ) );
    array_set(pos_array, 1, create_numberd(  current_window->event.dpos.y ) );
    array_set(pos_array, 2, create_numberd(  current_window->event.dpos.z ) );      
    return pos_array;
}

static NATIVECALL(layer_update){
    sendMessage(current_window, UPDATE , 0);
    return link_dup(This);
}

static NATIVECALL(layer_ident){
    Layer self   = This->value.vptr;
    if (self->transform){
        glmat_loadIdent(self->transform);
    }
    return link_dup(This);
}



NativeType layer_init(INITFUNC_ARGS){
    
    bound_layers = index_new(32);
    
    NativeType t    = newNative();
    child_manager(t);
    t->lib          = lib;
    t->destroy      = destroy;
    t->draw         = layer_draw;

    addNativeCall(t, "bias",           layer_bias);
    addNativeCall(t, "move",           layer_move);
    addNativeCall(t, "translate",      layer_translate);
    addNativeCall(t, "rotate",         layer_rotate);
    addNativeCall(t, "scale",          layer_scale);
    addNativeCall(t, "ident",          layer_ident);
    addNativeCall(t, "bind",           layer_bind);
    addNativeCall(t, "eventPosition",  layer_eventPosition);
    addNativeCall(t, "eventPos",  layer_eventPos);
    addNativeCall(t, "eventLayerPos",  layer_eventLayerPos);
    addNativeCall(t, "eventDPosition", layer_eventDPosition);
    addNativeCall(t, "eventWinPosition", layer_eventWinPosition);
    addNativeCall(t, "eventDWinPosition", layer_eventDWinPosition);
    addNativeCall(t, "eventDescription", layer_eventDescription);
    addNativeCall(t, "transform", layer_transform_apply);
    
    addNativeCall(t, "update", layer_update);
    
    addCFunc(Module, "layer", layer_new);
    return t;
}




