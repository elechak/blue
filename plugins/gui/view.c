/*
-------------------------------------------
Copyright 2005-2008 Erik Lechak
-------------------------------------------


This file is part of blue.

Bat is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Bat is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bat; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "graphics.h"

static inline void setBounds(View self, GLdouble * bounds){
    // Vertical origin  - bottom, top
    switch(self->origin & 0x3){
        case 0: //Defined
            bounds[2] = self->bounds[2]; // bottom
            bounds[3] = self->bounds[3]; // top
            break;
        case 1: // North
            bounds[2] = -1.0 * (double)(current_window->height) ; // bottom
            bounds[3] = 0.0; // top
            break;
        case 2: // Center
            bounds[2] = (double)(current_window->height) / -2.0; // bottom
            bounds[3] = (double)(current_window->height)/ 2.0; // top
            break;
        case 3: // South
            bounds[2] = 0.0; // bottom
            bounds[3] = (double)(current_window->height); // top
            break;
    }

    // Horizontal origin  - left, right
    switch( (self->origin>>2) & 0x3){
        case 0: //Defined
            bounds[0] = self->bounds[0]; // left
            bounds[1] = self->bounds[1]; // right
            break;
        case 1: // East
            bounds[0] = -1.0 * current_window->width ; // left
            bounds[1] = 0.0; // right
            break;
        case 2: // Center
            bounds[0] = current_window->width / -2.0; // left
            bounds[1] = current_window->width/ 2.0; // right
            break;
        case 3: // West
            bounds[0] = 0.0; // left
            bounds[1] = (unsigned int)current_window->width; // right
            break;
    }

    // Near
    bounds[4] = (self->bounds[4]);

    // Far
    bounds[5] = (self->bounds[5]);
}


/* Transforms window to view coordinates */
void view_transform(View self){
    Event event = &(current_window->event);
    static GLdouble bounds[6]; // left right bottom top near far

    setBounds(self,bounds);
    
    glmat_loadIdent(event->transform);

    GLdouble scalex = event->transform[0] = (bounds[1] - bounds[0])/current_viewport[2];
    GLdouble scaley = event->transform[5] = (bounds[3] - bounds[2])/current_viewport[3];

    event->transform[12] = -bounds[0];
    event->transform[13] = -bounds[2];
    
    event->dpos.x *=  scalex;
    event->dpos.y *=  scaley;

    event->pos.x = (event->pos.x * scalex)  + bounds[0];
    event->pos.y = (event->pos.y * scaley)  + bounds[2];
}


void view_draw(Link link){

    unsigned long draw_status_save;

    View self = current_view =  link->value.vptr;

    GLdouble mod_mat[16];
    static GLdouble bounds[6];

    glMatrixMode (GL_PROJECTION); // Set up projection view
    glLoadIdentity ();

    if (draw_status & SELECT){        
        glTranslatef( (current_viewport[2] - 2 * (pick_x - current_viewport[0])) / SELECTION_SIZE,   (current_viewport[3] - 2 * (pick_y - current_viewport[1])) / SELECTION_SIZE, 0);
        glScalef(current_viewport[2] / SELECTION_SIZE, current_viewport[3] / SELECTION_SIZE, 1.0);        
    }

    setBounds(self, bounds); 
    
    // Perspective or Orthographic projection
    if (self->projection){
        // left right bottom top near far
        glOrtho(bounds[0],bounds[1],bounds[2],bounds[3],bounds[4],bounds[5]);
    }else{
        // left right bottom top near far
        glFrustum(bounds[0],bounds[1],bounds[2],bounds[3],bounds[4],bounds[5]);
    }

    
    if (self->camera_pos){
        glTranslated(self->camera_pos[0],self->camera_pos[1], self->camera_pos[2]);
    }
    
    if (!(draw_status & SELECT)){
        // Store the value of the projection matrix while rendering
        glGetDoublev(GL_PROJECTION_MATRIX, self->proj_mat_render);
    }

    glGetDoublev(GL_PROJECTION_MATRIX, self->proj_mat);
    
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);

    raster_pos_select[0] = 0.0;
    raster_pos_select[1] = 0.0;
    raster_pos_select[2] = 0.0;

    glRasterPos2f( (bounds[0]+bounds[1])/2.0 ,(bounds[2]+bounds[3])/2.0);
    glGetDoublev(GL_CURRENT_RASTER_POSITION, raster_pos);

    if (self->fog_parameters[0]){
        glFogi(GL_FOG_MODE, GL_LINEAR); // Fog Mode
        glFogfv(GL_FOG_COLOR, current_window->clear_color); // Set Fog Color to clear color
        glFogf(GL_FOG_DENSITY, self->fog_parameters[0]);    // How Dense Will The Fog Be
        glHint(GL_FOG_HINT, GL_NICEST);			// Fog Hint Value
        glFogf(GL_FOG_START, self->fog_parameters[1]);  // Fog Start Depth
        glFogf(GL_FOG_END, self->fog_parameters[2]);    // Fog End Depth
        glEnable(GL_FOG);  // Enables GL_FOG
    }else{
        glDisable(GL_FOG);
    }

    // Reset light_index to 0 , everytime it encounters a light it increments the light index
    // So first light it runs into is GL_LIGHT0
    int c;
    for(c=0; c < light_index ; c++) glDisable(trans[c]);
    light_index = 0;
    glDisable(GL_LIGHTING);

    // DRAW
    // remember some important states
    draw_status_save = draw_status;
    glPushAttrib(  GL_ALL_ATTRIB_BITS );

    im_draw(self->items);

    // return saved states
    glPopAttrib(); // make sure material, lighting, and texture don't propagate up
    draw_status = draw_status_save;
}


static NATIVECALL(view_setBounds){
    fflush(stdout);
    View self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    self->bounds[0] = object_asNumber(args[0]);
    self->bounds[1] = object_asNumber(args[1]);
    self->bounds[2] = object_asNumber(args[2]);
    self->bounds[3] = object_asNumber(args[3]);
    self->bounds[4] = object_asNumber(args[4]);
    self->bounds[5] = object_asNumber(args[5]);

    return link_dup(This);
}

static NATIVECALL(view_setOrigin){
    View self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);

    string_t s = object_getString(args[0]);
    unsigned char origin = 0;


        switch( s->data[0] ){
            case 'n':
            case 'N':
                origin |= 1;
                break;
            case 'c':
            case 'C':
                origin |= 2;
                break;
            case 's':
            case 'S':
                origin |= 3;
                break;
        }

        switch( s->data[1] ){
            case 'e':
            case 'E':
                origin |= 1<<2;
                break;
            case 'c':
            case 'C':
                origin |= 2<<2;
                break;
            case 'w':
            case 'W':
                origin |= 3<<2;
                break;
        }

    self->origin = origin;

    return link_dup(This);
}



static void destroy(Link link){
    View self  = link->value.vptr;
    im_free( self->items);
    free(self);
}


static NATIVECALL(view_perspective){
    View self   = This->value.vptr;
    self->projection = 0;
    return object_create(Global->null_type);
}

static NATIVECALL(view_ortagonal){
    View self   = This->value.vptr;
    self->projection = 1;
    return object_create(Global->null_type);
}

static NATIVECALL(view_cameraPosition){
    View self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    
    if (! self->camera_pos) self->camera_pos = malloc( sizeof(GLdouble) *3);
    self->camera_pos[0] = object_asNumber(args[0]);
    self->camera_pos[1] = object_asNumber(args[1]);
    self->camera_pos[2] = object_asNumber(args[2]);
    return object_create(Global->null_type);
}


Link create_view(){
    View self = malloc(sizeof(*self));

    self->items = im_new();

    self->fog_parameters[0]=0.0; //Densisty
    self->fog_parameters[1]=0.0; //Start
    self->fog_parameters[2]=0.0; //End

    self->origin = 0;

    //left,  right, bottom, top, near, far
    self->bounds[0] = 0.0;
    self->bounds[1] = 100.0;
    self->bounds[2] = 0.0;
    self->bounds[3] = 100.0;
    self->bounds[4] = 0.0;
    self->bounds[5] = 100.0;

    self->projection = 1;//  0 is Perspective  , 1 is Orthographic

    self->camera_pos = NULL;
    
    //~ return lnk;
    Link obj = object_create(view_type);
    obj->value.vptr = self;

    return obj;
}



static NATIVECALL(shape_new){
    Link link = create_shape();
    View v = This->value.vptr;
    im_append(v->items, link);
    return link;
}

static NATIVECALL(image_new){
    Link link = create_image(Arg);
    View v = This->value.vptr;
    im_append(v->items, link);
    return link;
}

static NATIVECALL(layer_new){
    Link link = create_layer();
    View v = This->value.vptr;
    im_append(v->items, link);
    return link;
}

static NATIVECALL(text_new){
    Link link = create_text(Arg);
    View v = This->value.vptr;
    im_append(v->items, link);
    return link;
}






NativeType view_init(INITFUNC_ARGS){
    NativeType t    = newNative();
    t->lib          = lib;
    t->destroy      = destroy;
    t->draw         = view_draw;

    addNativeCall(t, "bounds", view_setBounds);
    addNativeCall(t, "origin", view_setOrigin);
    addNativeCall(t, "perspective", view_perspective );
    addNativeCall(t, "orthagonal", view_ortagonal);
    addNativeCall(t, "cameraPosition", view_cameraPosition);
    
    addNativeCall(t, "shape", shape_new);
    addNativeCall(t, "image", image_new);
    addNativeCall(t, "text", text_new);
    addNativeCall(t, "layer", layer_new);
    

    return t;
}


