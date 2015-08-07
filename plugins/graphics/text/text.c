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

#include "../graphics.h"

#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static NativeType text_type = NULL;

static FT_Library    library;
static FT_Error      error;
static FT_Face default_face = NULL;
static int default_size     = 0;


struct Text{
    string_t string;
    FT_Face face;
    int size;
    GLdouble * position;
    int raster_width;
    int raster_height;
    Binding bindings;
};
typedef struct Text * Text;


static void  create(Link link){
    Text self = link->value.vptr = malloc(sizeof(*self));
    self->string        = NULL;
    self->face          = NULL;
    self->size          = 0;
    self->position      = NULL;
    self->raster_width  = 0;
    self->raster_height = 0;
    self->bindings      = NULL;
}

static NATIVECALL(text_new){
    Link * args = array_getArray(Arg);
    Link link = object_create(text_type);
    Text self = link->value.vptr;
    self->string = string_dup(object_getString(args[0]));
    return link;
}



static GLdouble left;

void text_select(Link link){
    Binding binding;
    Text self = link->value.vptr;

    int size=default_size;
    if (self->size) size = self->size;

    if (self->position){
        rpSelectMoveToPos(self->position[0],self->position[1],self->position[2]);
        rpMove(0.0 , -size);
        left = raster_pos[0];
    }

    //printf("left %f\n", left);

    if (pick_x < left) return; // x to the left of text
    if (pick_y > raster_pos[1]+size) return; // y above text

    FT_Face face;
    if (self->face)
        face = self->face;
    else
        face = default_face;

    error = FT_Set_Pixel_Sizes(face, 0, size);
        if (error)  printf("could not set pixel size\n");

    char * text = self->string->data;
    FT_GlyphSlot slot = face->glyph;


    /* get onto the right line */
    while (pick_y < raster_pos[1]){
        while(*text && (*text != '\n') ){
            error = FT_Load_Char( face, *text, FT_LOAD_RENDER );
            rpMove(slot->bitmap_left, slot->bitmap_top);
            rpMove((slot->advance.x>>6) - slot->bitmap_left  , -1 * slot->bitmap_top);
            text++;
        }
        if (! *text) return;
        rpMoveTo(left, raster_pos[1] -size);
        text++;
    }

    while(*text){
        //printf("<%c>\n", *text);
        if (*text =='\n') break;
        //printf("rp %f   %f\n", raster_pos[0], left);

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( face, *text, FT_LOAD_RENDER );

        /* DRAW */
            if (pick_x > raster_pos[0]+ (slot->advance.x>>6)){
                rpMove(slot->bitmap_left, slot->bitmap_top);
                rpMove((slot->advance.x>>6) - slot->bitmap_left  , -1 * slot->bitmap_top);
            }else{
                forceHit();
                raster_hit = 1;
                binding = self->bindings;
                while(binding){
                    if( (binding->event.state & current_window->event.state) == binding->event.state){
                        callback(link,binding->callback);
                    }
                    binding = binding->next;
                }
                return;
            }
        text++;
    }


    while(*text){
        if (*text =='\n'){
            rpMoveTo(left, raster_pos[1] -size);
            text++;
        }
        error = FT_Load_Char( face, *text, FT_LOAD_RENDER );
        rpMove(slot->bitmap_left, slot->bitmap_top);
        rpMove((slot->advance.x>>6) - slot->bitmap_left  , -1 * slot->bitmap_top);
        text++;
    }

}




void text_draw(Link link){

    if (draw_status & SELECT){
        if (raster_hit) return;
        text_select(link);
        return;
    }
    Text self = link->value.vptr;

    /* set font size */
    int size=default_size;
    if (self->size) size = self->size;


    if (self->position){
        rpMoveToPos(self->position[0],self->position[1],self->position[2]);
        rpMove(0.0 , -size);
        left = raster_pos[0];
    }

    /* set font face */
    FT_Face face;
    if (self->face)
        face = self->face;
    else
        face = default_face;

    error = FT_Set_Pixel_Sizes(face, 0, size);
        if (error)  printf("could not set pixel size\n");

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glPixelZoom( 1.0, -1.0 );
    glEnable(GL_BLEND);

    char * text = self->string->data;
    FT_GlyphSlot slot = face->glyph;

    while(*text){

        if (*text == '\n'){
            rpMoveTo(left, raster_pos[1] -size);
            text++;
            continue;
        }

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( face, *text, FT_LOAD_RENDER );
            if ( error ) {
                printf("could not load char\n");
                text++;
                continue;
            }

        /* DRAW */
            rpMove(slot->bitmap_left, slot->bitmap_top);
            glDrawPixels(slot->bitmap.width,slot->bitmap.rows,GL_ALPHA,GL_UNSIGNED_BYTE,(GLubyte *)(slot->bitmap.buffer));
            rpMove((slot->advance.x>>6) - slot->bitmap_left  , -1 * slot->bitmap_top);
        text++;
    }
    glDisable(GL_BLEND);
    glPixelZoom( 1.0, -1.0 );
}


static NATIVECALL(text_size){
    Link * args = array_getArray(Arg);
    Text self = This->value.vptr;
    self->size = (int)object_asNumber(args[0]);
    return link_dup(This);
};

static NATIVECALL(text_position){
    Link * args = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);
    Text self = This->value.vptr;
    if (! self->position){
        self->position = malloc(sizeof(GLdouble) *3);
    }
    
    self->position[0] =(GLdouble) object_asNumber(args[0]);
    self->position[1] =(GLdouble) object_asNumber(args[1]);
    self->position[2] =(GLdouble) object_asNumber(args[2]);

    return link_dup(This);
};


static NATIVECALL(text_bind){
    Text self   = This->value.vptr;
    Link * args  = array_getArray(Arg);

    string_t string_event = object_getString(args[0]);
    Binding binding = binding_new(string_event->data, link_dup(args[1]));

    binding->next = self->bindings;
    self->bindings = binding;

    return link_dup(This);
}


void init(INITFUNC_ARGS){
    FT_Init_FreeType( &library );

    default_size = 20;

    string_t resource_dir = string_new("/resource/ryanlerch_-_Tuffy.ttf");
    string_t default_face_location = string_new_add(Global->blue_location,resource_dir );
    free(resource_dir);


    /* create default face  */
    error = FT_New_Face( library, default_face_location->data, 0, &default_face );


    free(default_face_location);

    if (error) printf("could not load default face (error number %d)\n", error);

    text_type    = newNative();
        text_type->lib       = lib;
        text_type->create    = create;
        text_type->draw      = text_draw;

    addNativeCall(text_type,"size", text_size);
    addNativeCall(text_type,"position", text_position);
    addNativeCall(text_type, "bind", text_bind);
    
    addCFunc(Module, "new", text_new);

}










