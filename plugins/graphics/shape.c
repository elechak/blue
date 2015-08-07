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


static NATIVECALL(shape_new){
    return object_create(shape_type);
}

static void  create(Link link){
    Shape self = link->value.vptr = malloc(sizeof(*self));

    self->vtx          = list_new( sizeof(struct Xyz) );
    self->normals      = list_new( sizeof(struct Xyz) );
    self->colors       = list_new( sizeof(struct Rgba) );
    self->texcoords    = list_new( sizeof(struct Xy)  );

    self->texture      = NULL;

    self->polygon_mode = 0;
    self->blend_mode[0] = 4;
    self->blend_mode[1] = 5;

    self->line_size = 1.0;
    self->point_size = 1.0;

    self->points= 0;
    self->lines= 0;
    self->line_strip= 0;
    self->line_loop= 0;
    self->triangles= 0;
    self->triangle_strip= 0;
    self->triangle_fan= 0;
    self->quads= 0;
    self->quad_strip= 0;
    self->polygon= 0;
}

static void destroy(Link link){
    Shape self = link->value.vptr;
    if (! self) return;
    if (self->vtx) list_free(self->vtx);
    if (self->normals) list_free(self->normals);
    if (self->colors) list_free(self->colors);
    if (self->texcoords) list_free(self->texcoords);
    free(self);
}

#define SHAPE_VERT_ITER       register int c;\
                              size_t length=self->vtx->length;\
                              struct Xyz *verts     = (struct Xyz *)(self->vtx->data);\
                              for (c=0; c < length; c++)

#define SHAPE_VERTS_CALL       glVertex3dv((GLdouble *)(verts + c));
#define SHAPE_COLORS_CALL      glColor4dv((GLdouble *)(colors + c));
#define SHAPE_TEXCOORDS_CALL   glTexCoord2dv((GLdouble *)(texcoords + c));
#define SHAPE_NORMALS_CALL     glNormal3dv((GLdouble *)(normals + c));

#define SHAPE_COLORS_DEF     struct Rgba *colors     = (struct Rgba *)(self->colors->data);
#define SHAPE_TEXCOORDS_DEF  struct Xy  *texcoords = (struct Xy *)(self->texcoords->data);
#define SHAPE_NORMALS_DEF    struct Xyz *normals   = (struct Xyz *)(self->normals->data);

/* Calls glVertex over all vertices in shape */
static inline void stroke_(Shape self){
    SHAPE_VERT_ITER{
        SHAPE_VERTS_CALL
    }
}

/* Calls glVertex and glColor for all vertices */
static inline void stroke_c(Shape self){
    SHAPE_COLORS_DEF

    SHAPE_VERT_ITER{
        SHAPE_COLORS_CALL
        SHAPE_VERTS_CALL
    }
}

static inline void stroke_t(Shape self){
    SHAPE_TEXCOORDS_DEF

    SHAPE_VERT_ITER{
        SHAPE_TEXCOORDS_CALL
        SHAPE_VERTS_CALL
    }
}

static inline void stroke_tc(Shape self){
    SHAPE_TEXCOORDS_DEF
    SHAPE_COLORS_DEF

    SHAPE_VERT_ITER{
        SHAPE_COLORS_CALL
        SHAPE_TEXCOORDS_CALL
        SHAPE_VERTS_CALL
    }
}

static inline void stroke_n(Shape self){
    SHAPE_NORMALS_DEF

    SHAPE_VERT_ITER{
        SHAPE_NORMALS_CALL
        SHAPE_VERTS_CALL
    }
}

static inline void stroke_nc(Shape self){
    SHAPE_NORMALS_DEF
    SHAPE_COLORS_DEF

    SHAPE_VERT_ITER{
        SHAPE_NORMALS_CALL
        SHAPE_COLORS_CALL
        SHAPE_VERTS_CALL
    }
}

static inline void stroke_nt(Shape self){
    SHAPE_NORMALS_DEF
    SHAPE_TEXCOORDS_DEF

    SHAPE_VERT_ITER{
        SHAPE_TEXCOORDS_CALL
        SHAPE_NORMALS_CALL
        SHAPE_VERTS_CALL
    }
}

static inline void stroke_ntc(Shape self){
    SHAPE_NORMALS_DEF
    SHAPE_TEXCOORDS_DEF
    SHAPE_COLORS_DEF

    SHAPE_VERT_ITER{
        SHAPE_NORMALS_CALL
        SHAPE_TEXCOORDS_CALL
        SHAPE_COLORS_CALL
        SHAPE_VERTS_CALL
    }
}

typedef void (*stroke_func)(Shape);


static const stroke_func stroke_table[]={
    stroke_,
    stroke_c,
    stroke_t,
    stroke_tc,
    stroke_n,
    stroke_nc,
    stroke_nt,
    stroke_ntc
};


void shape_draw(Link link){

    GLenum format = 0;
    stroke_func stroke;
    size_t size;
    unsigned long stroke_mode=0;

    Shape self = link->value.vptr;

    // is there any point data to draw?
    if (! (size = self->vtx->length) ) return;

    /* save opengl states */
    glPushAttrib(  GL_ALL_ATTRIB_BITS ); 

    // Size of primatives
    glLineWidth(self->line_size); // Set line width
    glPointSize(self->point_size); // Set point size


    /* Normals */
    if (self->normals->length){
        /* is there a normal for every vertex */
        if (self->normals->length >= size){
            stroke_mode |= 1L<<2;
        }else{
        /* If not use the first normal for all vertices */
            glNormal3fv((float *) self->normals->data);
        }
        
    }else{
        /* if there are no normals don't use lighting */
        glDisable(GL_LIGHTING);
    }

    if (self->texture){
        Image image = self->texture->value.vptr;
        glEnable(GL_COLOR_MATERIAL);

        if (! image->texture_id){
            switch(image->format){
                case 1:
                    format = GL_ALPHA;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
            }

            image->texture_id = (GLuint)image;
            glBindTexture(GL_TEXTURE_2D , image->texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height, 0, format, GL_UNSIGNED_BYTE, image->data);
            goto show_tex;
        }else if (self->texcoords->length){
            glBindTexture(GL_TEXTURE_2D, image->texture_id);
            show_tex:
            stroke_mode |= 1L<<1;
            glEnable(GL_TEXTURE_2D);
            if (image->format == 4){
                glEnable(GL_BLEND);
                glBlendFunc(source_mode[image->blend_mode[0]],dest_mode[image->blend_mode[1]]);
            }else{
                glDisable(GL_BLEND);
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    if (self->colors->length){
        glEnable(GL_COLOR_MATERIAL);
        if (self->colors->length >= size){
            stroke_mode |= 1L;
        }else{
            glColor4dv((GLdouble *) self->colors->data);
        }
    }

    glBlendFunc(source_mode[self->blend_mode[0]],dest_mode[self->blend_mode[1]]);
    glEnable(GL_BLEND);

    stroke = stroke_table[stroke_mode];

    // Poly_mode 0 = fill   1= edges    2=points
    switch (self->polygon_mode){
        case 0:
            glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
            break;
        case 1:
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            break;
        default:
            glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
            break;
    }

    
    float specular[] = {1.0,1.0,1.0,1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0f);    

    if (self->points){glBegin(GL_POINTS);stroke(self);glEnd();}
    if (self->lines){glBegin(GL_LINES);stroke(self);glEnd();}
    if (self->line_strip){glBegin(GL_LINE_STRIP);stroke(self);glEnd();}
    if (self->line_loop){glBegin(GL_LINE_LOOP);stroke(self);glEnd();}
    if (self->triangles){glBegin(GL_TRIANGLES);stroke(self);glEnd();}
    if (self->triangle_strip){glBegin(GL_TRIANGLE_STRIP);stroke(self);glEnd();}
    if (self->triangle_fan){glBegin(GL_TRIANGLE_FAN);stroke(self);glEnd();}
    if (self->quads){glBegin(GL_QUADS);stroke(self);glEnd();}
    if (self->quad_strip){glBegin(GL_QUAD_STRIP);stroke(self);glEnd();}
    if (self->polygon){glBegin(GL_POLYGON);stroke(self);glEnd();}

    //glFlush();
    glPopAttrib(); //reset all OGL states
}


static NATIVECALL(shape_blendMode){
    Link * args = array_getArray(Arg);
    Shape self = This->value.vptr;
    self->blend_mode[0] = (int) object_asNumber(args[0]);
    self->blend_mode[1] = (int) object_asNumber(args[1]);

    return link_dup(This);
};

static NATIVECALL(shape_addVertex){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    size_t c = 0;

    struct Xyz v;

    while(1){
        if (argn < 3) break;
        v.x = object_asNumber( args[c] );
        c++;
        v.y = object_asNumber( args[c] );
        c++;
        v.z = object_asNumber( args[c] );
        c++;
        list_append(self->vtx, &v);
        argn-=3;
    }
    return link_dup(This);
}

static NATIVECALL(shape_addNormal){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    size_t c = 0;

    struct Xyz v;

    while(1){
        if (argn < 3) break;
        v.x = object_asNumber( args[c] );
        c++;
        v.y = object_asNumber( args[c] );
        c++;
        v.z = object_asNumber( args[c] );
        c++;
        list_append(self->normals, &v);
        argn-=3;
    }
    return link_dup(This);
}

static NATIVECALL(shape_addColor){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    size_t c = 0;

    struct Rgba v;

    while(1){
        if (argn < 4) break;
        v.r = object_asNumber( args[c] );
        c++;
        v.g = object_asNumber( args[c] );
        c++;
        v.b = object_asNumber( args[c] );
        c++;
        v.a = object_asNumber( args[c] );
        c++;
        list_append(self->colors, &v);
        argn-=4;
    }
    return link_dup(This);
}

static NATIVECALL(shape_addTexCoord){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    size_t c = 0;

    struct Xy v;

    while(1){
        if (argn < 2) break;
        v.x = object_asNumber( args[c] );
        c++;
        v.y = object_asNumber( args[c] );
        c++;
        list_append(self->texcoords, &v);
        argn-=2;
    }
    return link_dup(This);
}


#define SHAPE_TYPE(TYPE,VAR)   static NATIVECALL(shape_##TYPE){\
                                Shape self   = This->value.vptr;\
                                Link * args  = array_getArray(Arg);\
                                if( args[0]->type->is_true(args[0]) ){\
                                    self->VAR =1;\
                                }else{\
                                    self->VAR=0;\
                                }\
                                return link_dup(This);\
                                }

SHAPE_TYPE(points , points);
SHAPE_TYPE(lines , lines);
SHAPE_TYPE(lineStrip , line_strip);
SHAPE_TYPE(lineLoop , line_loop);
SHAPE_TYPE(triangles , triangles);
SHAPE_TYPE(triangleStrip , triangle_strip);
SHAPE_TYPE(triangleFan , triangle_fan);
SHAPE_TYPE(quads , quads);
SHAPE_TYPE(quadStrip , quad_strip);
SHAPE_TYPE(polygon , polygon);

static NATIVECALL(shape_pointsize){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    self->point_size = object_asNumber( args[0] );
    return link_dup(This);
}

static NATIVECALL(shape_texture){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    self->texture = link_dup( args[0] );
    return link_dup(This);
}

static NATIVECALL(shape_invnorms){
    Shape self   = This->value.vptr;

    struct Xyz * norms  = (struct Xyz *)self->normals->data;
    register long iter_index = self->normals->length;

    for( ; iter_index; ){
        iter_index--;
        norms[iter_index].x *= -1;
        norms[iter_index].y *= -1;
        norms[iter_index].z *= -1;
    }
    return link_dup(This);
}

static NATIVECALL(shape_calc_normals){
    Shape self = This->value.vptr;
    float *v1, *v2, *v3;
    float vector1[3],vector2[3];
    float norm[3];
    float magnitude;
    int index;

    list_clear(self->normals);
    index =0;
    while( index < self->vtx->length){
        v1= list_get(self->vtx,index+0);
        v2 =list_get(self->vtx,index+1);
        v3 = list_get(self->vtx,index+2);

        vector1[0] =v1[0] - v2[0];
        vector1[1] =v1[1] - v2[1];
        vector1[2] =v1[2] - v2[2];

        vector2[0] =v3[0] - v1[0];
        vector2[1] =v3[1] - v1[1];
        vector2[2] =v3[2] - v1[2];

        norm[0] = vector1[1]*vector2[2]  - vector1[2]*vector2[1];
        norm[1] = vector1[2]*vector2[0]  - vector1[0]*vector2[2];
        norm[2] = vector1[0]*vector2[1]  - vector1[1]*vector2[0];

        magnitude = sqrt(norm[0]*norm[0]+norm[1]*norm[1]+norm[2]*norm[2]);
        norm[0] /=magnitude;
        norm[1] /=magnitude;
        norm[2] /=magnitude;

        list_append(self->normals, norm);
        list_append(self->normals, norm);
        list_append(self->normals, norm);
        index +=3;
    }
    return link_dup(This);
}


static NATIVECALL(shape_translate){
    Shape self = This->value.vptr;
    
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);
    
    number_t x = object_asNumber( args[0] );
    number_t y = object_asNumber( args[1] );
    number_t z = object_asNumber( args[2] );
    
    struct Xyz * xyz = (struct Xyz *) self->vtx->data;
    int c;
    int v_length  = self->vtx->length;
    //struct Xyz * xyz = (struct Xyz *) self->vtx->data;
    if (v_length){
        for(c=0;c < v_length ; c++ ){
            xyz->x += x;
            xyz->y += y;
            xyz->z += z;
            xyz++;
        }
    }
    
    return link_dup(This);
}


static NATIVECALL(shape_rotate){
    Shape self = This->value.vptr;
    
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);
    
    number_t a = object_asNumber( args[0] );
    number_t x = object_asNumber( args[1] );
    number_t y = object_asNumber( args[2] );
    number_t z = object_asNumber( args[3] );

    if (!(self->vtx->length || self->normals->length))
        return exception("BadRotation",NULL,NULL) ;

    GLdouble R[16] , c,s,t, angle;
    GLdouble mag;

    mag = sqrt(x*x + y*y + z*z);
    if (mag == 0.0) return exception("BadRotation",NULL,NULL) ;

    x/=mag;
    y/=mag;
    z/=mag;

    angle=a*(3.14159265359/180.0);
    c=cos(angle);
    s=sin(angle);
    t=1.0-c;

    R[0] = t*x*x +c;
    R[1] =t*x*y - s*z;
    R[2] =t*x*z +s*y;
    R[3]=0.0;
    R[4] =t*x*y+s*z;
    R[5] =t*y*y+c;
    R[6] =t*y*z-s*x;
    R[7] = 0.0;
    R[8] =t*x*z-s*y;
    R[9] =t*y*z+s*x;
    R[10] =t*z*z+c;
    R[11] =0.0 ;
    R[12] =0.0 ;
    R[13] =0.0 ;
    R[14] =0.0 ;
    R[15] =1.0 ;

    //orthag_mat(R);
    glmat_orthag(R);

    int cc;
    GLdouble *fval;
    int length  = self->vtx->length;
    //rotate vertices
    if (length){
        fval =(GLdouble *) self->vtx->data;
        for(cc=0;cc < length ; cc++ ){
            glmat_apply3v(R,fval);
            //apply_mat(R, fval, fval+1, fval+2);
            fval+=3;
        }
    }

    length = self->normals->length;
    //rotate normals
    if (length){
        fval =(GLdouble *) self->normals->data;
        for(cc=0;cc < length ; cc++ ){
            glmat_apply3v(R,fval);
            //apply_mat(R, fval, fval+1, fval+2);
            fval+=3;
        }
    }
    return link_dup(This);
}


static NATIVECALL(shape_scale){
    Shape self = This->value.vptr;
    
    Link * args  = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);
    
    number_t x = object_asNumber( args[0] );
    number_t y = object_asNumber( args[1] );
    number_t z = object_asNumber( args[2] );
    
    int c;
    int v_length  = self->vtx->length;
    struct Xyz * xyz = (struct Xyz *) self->vtx->data;
    if (v_length){
        for(c=0;c < v_length ; c++ ){
            xyz->x *= x;
            xyz->y *= y;
            xyz->z *= z;
            xyz++;
        }
    }
    return link_dup(This);
}

static NATIVECALL(shape_polyMode){
    Shape self   = This->value.vptr;
    Link * args  = array_getArray(Arg);
    string_t s = NULL;
    s = object_getString( args[0] );
    
    if ( s ){
        
        if (string_compare_cstring( s , "surface")==0){ 
            self->polygon_mode = 0;
        }else if (string_compare_cstring( s , "mesh")==0){ 
            self->polygon_mode = 1;
        }else if (string_compare_cstring( s , "particle")==0){
            self->polygon_mode = 2;
        }else{
            return exception("Unknown Polygon Mode",NULL,NULL);
        }
        return link_dup(This);
    }
    
    self->polygon_mode =(int)object_asNumber(  args[0] );
    return link_dup(This);
}

NativeType shape_init(INITFUNC_ARGS){
    NativeType t    = newNative();

    t->lib          = lib;
    t->create       = create;
    t->destroy      = destroy;
    t->draw         = shape_draw;

    addNativeCall(t, "addVertex", shape_addVertex);
    addNativeCall(t, "addNormal", shape_addNormal);
    addNativeCall(t, "addColor", shape_addColor);
    addNativeCall(t, "addTexCoord", shape_addTexCoord);
    addNativeCall(t, "addTexture", shape_texture);
    addNativeCall(t, "invNorms", shape_invnorms);
    addNativeCall(t, "calcNorms", shape_calc_normals);
    addNativeCall(t, "polyMode", shape_polyMode);
    addNativeCall(t, "blendMode", shape_blendMode);

    addNativeCall(t, "points",        shape_points);
    addNativeCall(t, "lines",         shape_lines);
    addNativeCall(t, "lineStrip",     shape_lineStrip);
    addNativeCall(t, "lineLoop",      shape_lineLoop);
    addNativeCall(t, "triangles",     shape_triangles);
    addNativeCall(t, "triangleStrip", shape_triangleStrip);
    addNativeCall(t, "triangleFan",   shape_triangleFan);
    addNativeCall(t, "quads",         shape_quads);
    addNativeCall(t, "quadStrip",     shape_quadStrip);
    addNativeCall(t, "polygon",       shape_polygon);
    
    addNativeCall(t, "scale",           shape_scale);
    addNativeCall(t, "translate",       shape_translate);
    addNativeCall(t, "rotate",          shape_rotate);
    
    addNativeCall(t, "pointSize",          shape_pointsize);

    addCFunc(Module, "shape", shape_new);
    
    

    

    return t;
}
















