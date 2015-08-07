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

#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library    library;
static FT_Error      error;
static FT_Face default_face = NULL;
static int default_size     = 20;


struct Text{
    string_t string;
    FT_Face face;
    int size;
    GLdouble * position;
    int select_index;
};
typedef struct Text * Text;


static void  create(Link link){
    Text self = link->value.vptr = malloc(sizeof(*self));
    self->string        = NULL;
    self->face          = default_face;
    self->size          = default_size;
    self->position      = NULL;
    self->select_index  = -1;
}

Link create_text(Link Arg){
    Link * args = array_getArray(Arg);
    Link link = object_create(text_type);
    Text self = link->value.vptr;
    self->string = string_dup(object_getString(args[0]));
    return link;
}

static GLdouble left;

void text_select(Text self){

    int size = self->size;
    FT_Face face = self->face;
    
    if (pick_x < left) return; // x to the left of text
    if (pick_y > raster_pos[1]+size) return; // y above text

    error = FT_Set_Pixel_Sizes(face, 0, size);
        if (error)  printf("could not set pixel size\n");

    char * text = self->string->data;
    FT_GlyphSlot slot = face->glyph;


    /* get onto the right line */
    while (pick_y < raster_pos[1]){
        while(*text && (*text != '\n') ){
            error = FT_Load_Char( face, *text, FT_LOAD_RENDER );
            rpMove( slot->advance.x>>6   ,    0 );
            text++;
        }
        if (! *text) return;
        rpMoveTo(left, raster_pos[1] -size);
        text++;
    }

    /* look for character hit */
    while(*text){
        if (*text =='\n') break;
        error = FT_Load_Char( face, *text, FT_LOAD_RENDER );

        if (pick_x > raster_pos[0]+ (slot->advance.x>>6)){
            rpMove( slot->advance.x>>6   ,    0 );
        }else{
            self->select_index = text - self->string->data;
            forceHit();
            return;
        }
        text++;
    }

    /* complete the raster */
    while(*text){
        if (*text =='\n'){
            rpMoveTo(left, raster_pos[1] -size);
            text++;
        }
        error = FT_Load_Char( face, *text, FT_LOAD_RENDER );
        rpMove( slot->advance.x>>6   ,    0 );
        text++;
    }
}


void text_paint(Text self){
    
    int size = self->size;
    FT_Face face = self->face;

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
            rpMoveTo(left, raster_pos[1] - size);
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
    glPixelZoom( 1.0, 1.0 );
}



void text_draw(Link link){
    Text self = link->value.vptr;
    
    if (self->position){
        rpSelectMoveToPos(self->position[0],self->position[1],self->position[2]);
        rpMove(0.0 , - self->size);
        left = raster_pos[0];
    }
    
    if (draw_status & SELECT){
        text_select(self);
    }else{
        text_paint(self);
    }
    
    return;
}

static NATIVECALL(text_value){
    Text self = This->value.vptr;
    return create_string(self->string->data);
};

static NATIVECALL(text_select_index){
    Text self = This->value.vptr;
    return create_numberi(self->select_index);
};

static NATIVECALL(text_size){
    Link * args = array_getArray(Arg);
    Text self = This->value.vptr;
    self->size = (int)object_asNumber(args[0]);
    return link_dup(This);
};

static NATIVECALL(text_position){
    Link * args = array_getArray(Arg);
    Text self = This->value.vptr;
    if (! self->position){
        self->position = malloc(sizeof(GLdouble) *3);
    }
    self->position[0] =(GLdouble) object_asNumber(args[0]);
    self->position[1] =(GLdouble) object_asNumber(args[1]);
    self->position[2] =(GLdouble) object_asNumber(args[2]);
    return link_dup(This);
};


NativeType text_init(INITFUNC_ARGS){
    FT_Init_FreeType( &library );

    string_t resource_dir = string_new("/resource/ryanlerch_-_Tuffy.ttf");
    string_t default_face_location = string_new_add(Global->blue_location,resource_dir );
    free(resource_dir);

    /* create default face  */
    error = FT_New_Face( library, default_face_location->data, 0, &default_face );

    free(default_face_location);

    if (error) fprintf(stderr,"could not load default face (error number %d)\n", error);

    text_type    = newNative();
        text_type->lib       = lib;
        text_type->create    = create;
        text_type->draw      = text_draw;

    addNativeCall(text_type,"size", text_size);
    addNativeCall(text_type,"position", text_position);
    addNativeCall(text_type,"select_index", text_select_index);
    addNativeCall(text_type,"value", text_value);
    
    return text_type;
}










