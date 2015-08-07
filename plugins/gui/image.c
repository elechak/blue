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

#include "graphics.h"

#include <png.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#define HEADERSIZE 4


Link png_image(string_t filename_string){

    unsigned char pngheader[HEADERSIZE];
    png_structp png;
    png_infop info;
    png_bytep * row_pointers;
    png_byte* row ;

    int  y, width, height;

    FILE * fp = fopen(filename_string->data , "rb");
        if (! fp) return exception("CouldNotOpenPNG", NULL, NULL);

    /* read the png header and ensure it is a png */
	fread(pngheader, 1, HEADERSIZE, fp);
	if (png_sig_cmp(pngheader, 0, HEADERSIZE)){
        return exception("NotPNG", NULL, NULL);
    }

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_set_sig_bytes(png, HEADERSIZE);
	info = png_create_info_struct(png);
	png_init_io(png, fp);
	png_read_info(png, info);
    
    Link obj = object_create(image_type);
    Image self = obj->value.vptr;
    self->width = info->width;
    self->height = info->height;
	width = info->width;
	height = info->height;

    /* read png information */
	png_read_update_info(png, info);

    /* allocate memory for rows */
	row_pointers = malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++) row_pointers[y] = malloc(info->rowbytes);

    /* read image into allocated memory */
	png_read_image(png, row_pointers);

    fclose(fp);

    png_byte * pdata;

    self->format = info->channels;
    self->data = pdata = malloc(self->width * self->height * self->format);
    for (y=0; y<height; y++) {
        row = row_pointers[height-y-1];
        memcpy(pdata,row, self->width * self->format);
        pdata+=self->width * self->format;
        free(row);
    }

    free(row_pointers);
    png_destroy_read_struct(&png, &info, NULL);

    return obj;
}


static string_t  png_descriptor;

Link create_image(Link Arg){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    string_t filename;
    
    if ((argn) &&( filename = object_getString(args[0]) )){
            if ( string_endsWith(filename, png_descriptor) ){
                return png_image(filename);
            }else{
                return exception("CouldNotLoadImage",NULL,NULL);
            }
    }
        
    Link obj = object_create(image_type);
    Image self = obj->value.vptr;

    if (argn==2){
        self->width = (size_t)object_asNumber(args[0]);
        self->height = (size_t)object_asNumber(args[1]);
        self->data = malloc(self->width * self->height * 4); // RGBA
        self->format = 4;
        memset(self->data, 255, self->width * self->height * 4);
    }
    
    return obj;
}




/* Raster Positioning Functions */

void rpMove(GLfloat x, GLfloat y){
    glBitmap(0, 0,0,0,x , y , NULL);
    raster_pos[0] +=x;
    raster_pos[1] +=y;
}

void rpMoveTo(GLdouble xdest, GLdouble ydest){
    GLdouble xdiff = xdest - raster_pos[0] ;
    GLdouble ydiff = ydest - raster_pos[1] ;
    glBitmap(0, 0,0,0, xdiff , ydiff , NULL);
    raster_pos[0] = xdest;
    raster_pos[1] = ydest;
}


void rpMoveToPos(GLdouble xdest, GLdouble ydest, GLdouble zdest){
    GLdouble mod_mat[16];
    GLdouble px,py,pz;
    // Get the model matrix
    glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);

    //translate desired position coordinate into windows coordinate    ----- where we want to be
    gluProject(xdest,ydest,zdest, mod_mat,current_view->proj_mat, current_viewport,&px,&py,&pz);

    rpMoveTo(px,py);
}

void rpSelectMoveToPos(GLdouble xdest, GLdouble ydest, GLdouble zdest){
    GLdouble mod_mat[16];
    GLdouble px,py,pz;
    // Get the model matrix
    glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);

    //translate desired position coordinate into windows coordinate    ----- where we want to be
    gluProject(xdest,ydest,zdest, mod_mat,current_view->proj_mat_render, current_viewport,&px,&py,&pz);

    rpMoveTo(px,py);
}





static void create(Link link){
    Image self = malloc(sizeof(*self));

    self->data = NULL;
    self->status = 0;
    self->width  = 0;
    self->height = 0;
    self->format = 0;
    self->position[0] =self->position[1] =self->position[2]=0.0;
    self->padding[0]=self->padding[1]=self->padding[2]=self->padding[3]=0;
    self->blend_mode[0] = 4;
    self->blend_mode[1] = 5;

    self->texture_id = 0;

    link->value.vptr = self;
}

static void destroy(Link link){
    Image self = link->value.vptr;
    if (! self) return;
    if (self->data) free(self->data);
    free(self);
}



//static const int vp[] = {0, 0,SELECTION_SIZE,SELECTION_SIZE};
void image_draw(Link link){

    int width ,height, length;
    GLdouble mod_mat[16], p1,p2,p3,p_mat[16];

    Image self = link->value.vptr;
    View view = current_view;

    width = self->width;
    height = self->height;
    length = width * height * self->format;

    // SELECT
    if (draw_status & SELECT){ // We are determining if we selected this image

        glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);

        //-------------------------------------------------------------------------
        // PRE-SELECTION
        //-------------------------------------------------------------------------
        if (self->status & RELATIVE_POS){
            // move to actual location of image

            p1 = raster_pos_select[0];
            p2 = raster_pos_select[1];
            p3 = raster_pos_select[2];

        }else{  //ABSOLUTE positioning

            glGetDoublev(GL_PROJECTION_MATRIX, p_mat);
            // since we are not actually drawing the image we have to move the simulated raster pos manually
            //translate our position coordinate into windows coordinate    ----- where we want to be
            gluProject(self->position[0],self->position[1],self->position[2], mod_mat,view->proj_mat_render, current_viewport,&p1,&p2,&p3);

            raster_pos_select[0] = p1;
            raster_pos_select[1] = p2;
            raster_pos_select[2] = p3;
        }

        //-------------------------------------------------------------------------
        //SELECTION
        //-------------------------------------------------------------------------

        //fprintf(stderr," (%i, %i) x=%f,%f  y=%f,%f \n",pick_x, pick_y, p1,p1+width, p2 , p2+height );

        if ( length && (pick_x > p1) && (pick_y > p2) && (pick_x < p1+width) && (pick_y < p2+height) ){
            forceHit();
        }

        //-------------------------------------------------------------------------
        // POST-SELECTION
        //-------------------------------------------------------------------------
        raster_pos_select[0] += self->padding[2];
        raster_pos_select[1] += self->padding[3];

        switch ((self->status & RELATIVE_POS)>>REL_OFFSET){
            case 1: // Rel-Y
                gluProject(self->position[0],self->position[1],self->position[2], mod_mat,view->proj_mat_render, current_viewport,&p1,&p2,&p3);
                raster_pos_select[1] = p2;
                break;
            case 2: // Rel-X
                gluProject(self->position[0],self->position[1],self->position[2], mod_mat,view->proj_mat_render, current_viewport,&p1,&p2,&p3);
                raster_pos_select[0] = p1;
                break;
        }



    // DRAW
    }else{ // We are drawing the image

        //-------------------------------------------------------------------------
        // PRE-DRAW
        //-------------------------------------------------------------------------
        if (self->status & RELATIVE_POS){
            rpMove(self->padding[0],self->padding[1]);
        }else{
            //ABSOLUTE positioning
            rpMoveToPos(self->position[0],self->position[1],self->position[2]);
        }

        //-------------------------------------------------------------------------
        // DRAW
        //-------------------------------------------------------------------------
        if (length && self->data){

            glPixelStorei(GL_UNPACK_ALIGNMENT,1);


            if (! (draw_status & MATERIAL)){
                glBlendFunc(source_mode[self->blend_mode[0]],dest_mode[self->blend_mode[1]]);
            }

            if (self->format == 4){
                glEnable(GL_BLEND);
                glDrawPixels(width,height,GL_RGBA,GL_UNSIGNED_BYTE,self->data);
                glDisable(GL_BLEND);
            }else if (self->format == 3){
                glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,self->data);
            }else if (self->format == 1){
                glEnable(GL_BLEND);
                glDrawPixels(width,height,GL_ALPHA,GL_UNSIGNED_BYTE,self->data);
                glDisable(GL_BLEND);
            }
        }

        //-------------------------------------------------------------------------
        // POST-DRAW
        //-------------------------------------------------------------------------

        rpMove(self->padding[2] - self->padding[0],self->padding[3]- self->padding[1]);

        switch ((self->status & RELATIVE_POS)>>REL_OFFSET){
            case 1: // Rel-Y
                rpMoveTo(raster_pos[0], p2);
                break;
            case 2: // Rel-X
                rpMoveTo(p1,raster_pos[1]);
                break;
        }
    }
}

static NATIVECALL(image_position){
    Link * args = array_getArray(Arg);
    Image self = This->value.vptr;
    self->position[0] = object_asNumber(args[0]);
    self->position[1] = object_asNumber(args[1]);
    self->position[2] = object_asNumber(args[2]);

    return link_dup(This);
};

static NATIVECALL(image_blendmode){
    Link * args = array_getArray(Arg);
    Image self = This->value.vptr;
    self->blend_mode[0] = (int) object_asNumber(args[0]);
    self->blend_mode[1] = (int) object_asNumber(args[1]);

    return link_dup(This);
};

NativeType image_init(INITFUNC_ARGS){
    NativeType t    = newNative();
    t->lib          = lib;
    t->create       = create;
    t->destroy      = destroy;
    t->draw         = image_draw;

    addNativeCall(t,"position", image_position);
    addNativeCall(t,"blendMode", image_blendmode);
        
    png_descriptor = string_new(".png");
    
    return t;
}
