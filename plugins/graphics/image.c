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

static NATIVECALL(image_new){
    Link obj = object_create(image_type);

    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

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

        //printf(" (%i, %i) x=%f,%f  y=%f,%f \n",pick_x, pick_y, p1,p1+width, p2 , p2+height );

        if ( length && (pick_x > p1) && (pick_y > p2) && (pick_x < p1+width) && (pick_y < p2+height) ){
            
            forceHit();
            
            //~ GLdouble point[4];
            //~ point[0] = pick_x;
            //~ point[1] = pick_y;
            
            //~ glmat_win2obj(point);
                        
            //~ glBegin(GL_POINTS);
                //~ glVertex3dv(point); // image is replaced by the glvertex3f call
            //~ glEnd();
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
    
    addCFunc(Module, "image", image_new);
    return t;
}


//~ void Image_move(Link lnk, float x, float y, float z){
    //~ Image self = OBJ(lnk, Image);

    //~ if (self->status & RELATIVE_POS){
        //~ self->padding[0] += (int)x;
        //~ self->padding[1] += (int)y;
    //~ }else{
        //~ self->position[0] += x;
        //~ self->position[1] += y;
        //~ self->position[2] += z;
    //~ }
//~ }



//~ void Image_scale(Link lnk, float sx, float sy, float sz){
    //~ Image self = OBJ(lnk, Image);
    //~ self->position[2] *= sz;
    //~ image_scaleto(self, (int)(self->size[0]*sx), (int)(self->size[1] *sy));
//~ }

//~ void image_scaleto(Image self, int width, int  height){

    //~ unsigned long format;
    //~ int length;
    //~ List temp;
    //~ List newimage = list_new(1);

    //~ switch (self->format){
        //~ case 3:
            //~ length = width * height *3;
            //~ format = GL_RGB;
            //~ break;
        //~ case 4:
            //~ length = width * height *4;
            //~ format = GL_RGBA;
            //~ break;
        //~ default:
            //~ return;
    //~ }

    //~ if  (! list_alloc(newimage,length*2) ) return;

    //~ // TODO: what the heck?
    //~ glPixelStorei(GL_PACK_ALIGNMENT,1);
    //~ glPixelStorei(GL_UNPACK_ALIGNMENT,1);


    //~ gluScaleImage(
        //~ format,
        //~ self->size[0],
        //~ self->size[1],
        //~ GL_UNSIGNED_BYTE,
        //~ self->image->data,
        //~ width,
        //~ height,
        //~ GL_UNSIGNED_BYTE,
        //~ newimage->data);

    //~ temp = self->image;
    //~ newimage->length = length;
    //~ self->image= newimage;
    //~ list_free(temp);
    //~ self->size[0] = width;
    //~ self->size[1] = height;
//~ }



//~ void image_vflip(Image self){
    //~ int c, half, width,height;

    //~ width = self->size[0] * self->format;

    //~ height = self->size[1];

    //~ unsigned char * temp = malloc(width);
    //~ if (! temp) return;

    //~ half = height / 2;


    //~ for (c=0; c<half; c++){
        //~ memcpy(temp,self->image->data + width * c, width);
        //~ memcpy(self->image->data + width * c , self->image->data + width * (height - c-1 ), width);
        //~ memcpy(self->image->data + width * (height - c-1), temp,width);
    //~ }
    //~ free(temp);
//~ }

//~ void image_hflip(Image self){
    //~ register int x;
    //~ register int y;
    //~ int half, width,height, rowsize, size;

    //~ width = self->size[0] ;
    //~ height = self->size[1];
    //~ size = self->format;
    //~ rowsize = width * size;
    //~ unsigned char * row = self->image->data;


    //~ unsigned char * temp = malloc(size);
    //~ if (! temp) return;

    //~ half = width / 2;

    //~ for (y=0; y<height; y++){
        //~ row += rowsize;
        //~ for (x=0; x<half; x++){
            //~ memcpy(temp,row + x*size, size);
            //~ memcpy(row + x*size ,(row + rowsize-size) - x * size , size);
            //~ memcpy((row + rowsize-size) - x * size, temp,size);
        //~ }
    //~ }
    //~ free(temp);
//~ }


//~ int Image_parse(Parser parser){

    //~ Image self = OBJ(CURRENT_LINK, Image);

    //~ List list = NULL;
    //~ unsigned char intVal;
    //~ int ints[3];

    //~ MATCH("rgba"){
        //~ self->format = 4;
        //~ list = self->image;

        //~ while(1){
            //~ stream_getToken(parser->stream,GARBAGE);
            //~ if (! isNumber(TOKEN) ) { PUTBACK; break;}
            //~ intVal = strtol(TOKEN,NULL,10);
            //~ list_append( list, &intVal);
        //~ }
    //~ } else MATCH("rgb"){
        //~ self->format = 3;
        //~ list = self->image;

        //~ while(1){
            //~ GETTOKEN(GARBAGE);
            //~ if (! isNumber(TOKEN) ) { PUTBACK; break;}
            //~ intVal = strtol(TOKEN,NULL,10);
            //~ list_append( list, &intVal);
        //~ }
    //~ } else MATCH("alpha"){
        //~ self->format = 1;
        //~ list = self->image;

        //~ while(1){
            //~ GETTOKEN(GARBAGE);
            //~ if (! isNumber(TOKEN) ) { PUTBACK; break;}
            //~ intVal = strtol(TOKEN,NULL,10);
            //~ list_append( list, &intVal);
        //~ }

    //~ }else MATCH("hide"){
        //~ CURRENT_LINK->status |= HIDDEN;

    //~ }else MATCH("show"){
        //~ CURRENT_LINK->status &= ~HIDDEN;


    //~ }else MATCH("vflip"){
        //~ image_vflip(self);

    //~ }else MATCH("hflip"){
        //~ image_hflip(self);

    //~ }else MATCH("scaleto"){
        //~ GETINT(2,ints);
        //~ image_scaleto(self,ints[0],ints[1]);
    //~ }else MATCH("padding"){
        //~ GETINT(4,self->padding);
    //~ }else MATCH("size"){
        //~ GETINT(2,self->size);
    //~ }else MATCH("position"){
        //~ GETFLOAT(3,self->position);
    //~ }else MATCH("blend"){
        //~ GETINT(2,self->blend_mode);
    //~ }else MATCH("relative"){
        //~ self->status |= RELATIVE_POS;
    //~ }else MATCH("relx"){
        //~ self->status &= ~RELATIVE_POS;
        //~ self->status |= RELATIVE_X;
    //~ }else MATCH("rely"){
        //~ self->status &= ~RELATIVE_POS;
        //~ self->status |= RELATIVE_Y;
    //~ }else MATCH("absolute"){
        //~ self->status &= ~RELATIVE_POS;

    //~ }else MATCH(DEF){
        //~ GETTOKEN(GARBAGE);
        //~ Parser_create(parser, CREATE(Var), TOKEN);

    //~ }else{
        //~ return 1; // Could not match token
    //~ }
    //~ return 0; // Everything went well
//~ }


//~ void Image_attrib(Parser parser){

    //~ Image self = OBJ(CURRENT_LINK, Image);

    //~ Parser_reply_writef(parser,"position %f %f %f \n", self->position[0] ,self->position[1] ,self->position[2],self->position[3] );
    //~ Parser_reply_writef(parser,"padding %i %i %i %i \n", self->padding[0] ,self->padding[1] ,self->padding[2],self->padding[3] );
    //~ Parser_reply_writef(parser,"size %i %i \n", self->size[0] ,self->size[1]);
    //~ Parser_reply_writef(parser,"blend %i %i \n", self->blend_mode[0] ,self->blend_mode[1]);

    //~ Parser_reply_write(parser,"format ");
    //~ if (self->format == 1)
        //~ Parser_reply_write(parser,"alpha\n");
    //~ else if (self->format == 3)
        //~ Parser_reply_write(parser,"rgb\n");
    //~ else if (self->format == 4)
        //~ Parser_reply_write(parser,"rgba\n");


    //~ if ( (self->status & RELATIVE_POS) == RELATIVE_POS){
        //~ Parser_reply_write(parser,"relative\n");
    //~ }else if ( (self->status & RELATIVE_POS) == RELATIVE_X){
        //~ Parser_reply_write(parser,"relx\n");
    //~ }else if ( (self->status & RELATIVE_POS) == RELATIVE_Y){
        //~ Parser_reply_write(parser,"rely\n");
    //~ }

    //~ if (self->format == 3)
        //~ Parser_reply_write(parser, "rgb\n");
    //~ else if (self->format == 4)
        //~ Parser_reply_write(parser, "rgba\n");
    //~ else if (self->format == 1)
        //~ Parser_reply_write(parser, "alpha\n");

    //~ if (self->format){
        //~ Parser_reply_writef(parser,"pixels %i \n", self->image->length / self->format);
    //~ }else{
        //~ Parser_reply_write(parser,"pixels 0 \n");
    //~ }
//~ }


//~ void Image_toStream(Link lnk, Stream stream){
    //~ SELF(Image);
    //~ unsigned char val;
    //~ register int iter_index;

        //~ stream_printf(stream,"position %f %f %f \n", self->position[0] ,self->position[1] ,self->position[2],self->position[3] );
        //~ stream_printf(stream,"padding %i %i %i %i \n", self->padding[0] ,self->padding[1] ,self->padding[2],self->padding[3] );
        //~ stream_printf(stream,"size %i %i \n", self->size[0] ,self->size[1]);
        //~ stream_printf(stream,"blend %i %i \n", self->blend_mode[0] ,self->blend_mode[1]);

        //~ if ( (self->status & RELATIVE_POS) == RELATIVE_POS){
            //~ stream_print(stream,"relative\n");
        //~ }else if ( (self->status & RELATIVE_POS) == RELATIVE_X){
            //~ stream_print(stream,"relx\n");
        //~ }else if ( (self->status & RELATIVE_POS) == RELATIVE_Y){
            //~ stream_print(stream,"rely\n");
        //~ }


        //~ if (self->format == 1)
            //~ stream_print(stream, "   alpha\n");
        //~ else if (self->format == 3)
            //~ stream_print(stream, "   rgb\n");
        //~ else if (self->format == 4)
            //~ stream_print(stream, "   rgba\n");

        //~ if (self->format){
            //~ list_iter(self->image,unsigned char,val){
                //~ stream_printf(stream,"%u ",val);
            //~ }
        //~ }

    //~ stream_print(stream, "\n");
//~ }








