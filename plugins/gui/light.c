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


static NATIVECALL(light_new){
    return object_create(light_type);
}

static void  create(Link link){
    Light self = link->value.vptr = malloc(sizeof(*self));
    
    self->ambient   = NULL;
    self->diffuse   = NULL;
    self->specular  = NULL;
    
    self->position[0]=self->position[1]=self->position[2]=0.0 ;
    self->position[3]=1.0;
    self->spot_direction[0]=self->spot_direction[1]=0.0;
    self->spot_direction[2]=-1.0;
    self->spot_exponent=0.0;
    self->spot_cutoff=180.0;
    self->constant_attenuation=1.0;
    self->linear_attenuation=0.0;
    self->quadratic_attenuation=0.0;    
}

static void destroy(Link link){
    Light self = link->value.vptr;
    if (! self) return;
    
    if ( self->ambient ) free(self->ambient);
    if ( self->diffuse ) free(self->diffuse);
    if ( self->specular) free(self->specular);
    free(self);    
}

void light_draw(Link lnk){

    Light self = lnk->value.vptr;

    glEnable(GL_LIGHTING);
    //glEnable(GL_NORMALIZE);
    
    //~ if (self->ambient){
        //~ DPRINT("%g  %g  %g  %g\n", self->ambient[0],self->ambient[1],self->ambient[2],self->ambient[3])
    //~ }
    
    if ( self->ambient ) glLightfv(trans[light_index], GL_AMBIENT, self->ambient);
    if ( self->diffuse ) glLightfv(trans[light_index], GL_DIFFUSE, self->diffuse);
    if ( self->specular) glLightfv(trans[light_index], GL_SPECULAR, self->specular);

    glLightfv(trans[light_index], GL_POSITION, self->position);
   // glLightfv(trans[light_index], GL_SPOT_DIRECTION, self->spot_direction);
    //glLightf(trans[light_index], GL_SPOT_EXPONENT, self->spot_exponent);
    //glLightf(trans[light_index], GL_SPOT_CUTOFF, self->spot_cutoff);
    //glLightf(trans[light_index], GL_CONSTANT_ATTENUATION, self->constant_attenuation);
    //glLightf(trans[light_index], GL_LINEAR_ATTENUATION, self->linear_attenuation);
    //glLightf(trans[light_index], GL_QUADRATIC_ATTENUATION, self->quadratic_attenuation);

    glEnable(trans[light_index]);
    light_index++;
}



static NATIVECALL(light_ambient){
    Light self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    
    if (! self->ambient){
        self->ambient = malloc( sizeof(float) * 4);
        self->ambient[3] = 1.0;
    }
    
    self->ambient[0] = object_asNumber(args[0]);
    self->ambient[1] = object_asNumber(args[1]);
    self->ambient[2] = object_asNumber(args[2]);
    return link_dup(This);
}


static NATIVECALL(light_diffuse){
    Light self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    
    if (! self->diffuse){
        self->diffuse = malloc( sizeof(float) * 4);
        self->diffuse[3] = 1.0;
    }
    
    self->diffuse[0] = object_asNumber(args[0]);
    self->diffuse[1] = object_asNumber(args[1]);
    self->diffuse[2] = object_asNumber(args[2]);
    return link_dup(This);
}


static NATIVECALL(light_specular){
    Light self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    
    if (! self->specular){
        self->specular = malloc( sizeof(float) * 4);
        self->specular[3] = 1.0;
    }
    
    self->specular[0] = object_asNumber(args[0]);
    self->specular[1] = object_asNumber(args[1]);
    self->specular[2] = object_asNumber(args[2]);
    return link_dup(This);
}



static NATIVECALL(light_move){
    Light self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    self->position[0] += object_asNumber(args[0]);
    self->position[1] += object_asNumber(args[1]);
    self->position[2] += object_asNumber(args[2]);
    return link_dup(This);
}

static NATIVECALL(light_position){
    Light self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    self->position[0] = object_asNumber(args[0]);
    self->position[1] = object_asNumber(args[1]);
    self->position[2] = object_asNumber(args[2]);
    return link_dup(This);
}






NativeType light_init(INITFUNC_ARGS){
    NativeType t    = newNative();

    t->lib          = lib;
    t->create       = create;
    t->destroy      = destroy;
    t->draw         = light_draw;

    addCFunc(Module, "light", light_new);
    
    addNativeCall(t, "move", light_move);
    addNativeCall(t, "position", light_position);
    
    addNativeCall(t, "ambient", light_ambient);
    addNativeCall(t, "diffuse", light_diffuse);
    addNativeCall(t, "specular", light_specular);
    
    

    return t;
}

