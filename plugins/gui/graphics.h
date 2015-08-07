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

#ifndef _GRAPHICS
#define _GRAPHICS


#include "../../global.h"
#include "glmat.h"

#include "lists.h"
#include "../../threading.h"
#include "../../index.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

EXPORT void init(INITFUNC_ARGS);
void blue_gl_init(INITFUNC_ARGS);

NativeType png_init(INITFUNC_ARGS);
NativeType text_init(INITFUNC_ARGS);




Link create_view();
Link create_shape();
Link create_image(Link Arg);
Link create_layer();
Link create_text(Link Arg);

#define SELECTION_SIZE 1

//Window messages
//---------------------------------------------------------
#ifdef __WIN32__
#define MESSAGE WM_USER+
#else
#define MESSAGE
#endif

#define UPDATE      MESSAGE 1
#define TITLE       MESSAGE 2
#define VISIBILITY  MESSAGE 3
#define FREE        MESSAGE 4
#define RESIZE      MESSAGE 5
#define POSITION    MESSAGE 6
#define DECOR       MESSAGE 7
#define CREATE      MESSAGE 8



#ifdef __WIN32__
#include <windows.h>        // Header File For Windows
#include <windowsx.h>        // Header File For Windows
#define WinHandle HWND
#define Context HGLRC

#else
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>
#define WinHandle Window
#define Context GLXContext
#endif



struct Xy{
    number_t x;
    number_t y;
};

struct Xyz{
    number_t x;
    number_t y;
    number_t z;
};

struct Rgba{
    number_t r;
    number_t g;
    number_t b;
    number_t a;
};

struct Event{
    unsigned long state;
    unsigned long key;
    struct Xyz pos;
    struct Xyz dpos;
    GLdouble transform[16];
};
typedef struct Event * Event;


struct ItemManager{
    Link * items;
    int size;
    int capacity;
};
typedef struct ItemManager * ItemManager;

ItemManager im_new();
void im_free( ItemManager self);
void im_append( ItemManager self, Link item);
void im_draw(ItemManager self);
    
    
#define BEVT_DELIM " -+\t\n\r"

#define BEVT_SHIFT          1 << 0
#define BEVT_SHIFTLOCK      1 << 1
#define BEVT_CTRL           1 << 2
#define BEVT_ALT            1 << 3
#define BEVT_NUMLOCK        1 << 4
#define BEVT_WIN            1 << 5
#define BEVT_KEY            1 << 6
#define BEVT_MOUSE          1 << 7
#define BEVT_MOUSE1         1 << 8
#define BEVT_MOUSE2         1 << 9
#define BEVT_MOUSE3         1 << 10
#define BEVT_MOUSE4         1 << 11
#define BEVT_MOUSE5         1 << 12
#define BEVT_PRESS          1 << 13
#define BEVT_RELEASE        1 << 14
#define BEVT_MOTION         1 << 15
#define BEVT_CLOSE          1 << 16
#define BEVT_WHEEL          1 << 17

struct Binding{
    struct Event event;
    Link callback;
    struct Binding * next;
};
typedef struct Binding * Binding;
EXPORT Binding binding_new(char * string, Link callback);



/* BWindow */
struct BWindow{
    ItemManager items;
    unsigned int width;
    unsigned int height;
    int x;
    int y;
    float clear_color[4];
    string_t title;
    unsigned int depth;
    WinHandle hwindow;
    Context context;
    struct Event event;
    Binding bindings;
    int update;
    
    Link onClose;
    Link onKeyDown;
    Link onKeyUp;
};

typedef struct BWindow * BWindow;
NativeType window_init(INITFUNC_ARGS);
void bwindow_create(Link link);
void bwindow_free(BWindow self);
void bwindow_draw(BWindow self);
void bwindow_pick(BWindow self);
EXPORT void forceHit();

/* View */
struct View{
    ItemManager items;    
    GLdouble bounds[6];
    GLdouble fog_parameters[3]; // density, start,end
    GLdouble proj_mat[16];
    GLdouble proj_mat_render[16];
    unsigned int projection; // TODO: this could be in object.status
    unsigned char origin; // horizontal (North, Center, South,Defined),  vert(East, Center, West, Definde) i.e SW
    GLdouble  *camera_pos;
};
typedef struct View * View;
NativeType view_init(INITFUNC_ARGS);
void view_transform(View self);

/* Layer */
struct Layer{
    ItemManager items;
    GLdouble * transform;
    Binding bindings;
    float * bias;
    GLuint bound_id;
};
typedef struct Layer * Layer;

extern Index bound_layers;

NativeType layer_init(INITFUNC_ARGS);
void layer_transform(Layer self);


/* Shape */
struct Shape{
    List vtx;
    List normals;
    List colors;
    List texcoords;
    Link texture;
    int weight;
    float line_size;
    float point_size;
    int polygon_mode;
    int blend_mode[2]; // can be char
    //unsigned textured: 1;
    //unsigned smooth: 1;
    unsigned points: 1;
    unsigned lines: 1;
    unsigned line_strip: 1;
    unsigned line_loop: 1;
    unsigned triangles: 1;
    unsigned triangle_strip: 1;
    unsigned triangle_fan: 1;
    unsigned quads: 1;
    unsigned quad_strip: 1;
    unsigned polygon: 1;



};
typedef struct Shape * Shape;

NativeType shape_init(INITFUNC_ARGS);

// IMAGE
struct Image{
    unsigned char * data;
    int width;
    int height;
    int format; // 4=rgba  3=rgb  1=alpha
    int blend_mode[2]; // can be char
    float position[3];
    int padding[4];
    unsigned long status;
    GLuint texture_id;
};
typedef struct Image * Image;
EXPORT void rpMove(GLfloat x, GLfloat y);
EXPORT void rpMoveTo(GLdouble xdest, GLdouble ydest);
EXPORT void rpMoveToPos(GLdouble xdest, GLdouble ydest, GLdouble zdest);
EXPORT void rpSelectMoveToPos(GLdouble xdest, GLdouble ydest, GLdouble zdest);
NativeType image_init(INITFUNC_ARGS);
#define REL_OFFSET    4
#define RELATIVE_Y   0x1<< 4
#define RELATIVE_X   0x2<< 4
#define RELATIVE_POS 0x3<< 4


/* LIGHT */
struct Light{
    float *ambient;
    float *diffuse;
    float *specular;
    float position[4];
    float spot_direction[3];
    float spot_exponent;
    float spot_cutoff;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};
typedef struct Light * Light;
NativeType light_init(INITFUNC_ARGS);


/* sends message/event to GUI dependant event loop   - defined in graphics.c or graphics_win32.c */
void sendMessage(void * vp, unsigned long type , unsigned long message);

/* COMMON   see top of window.c for definition*/
#define MATERIAL 0x1<<0
#define TEXTURE  0x1<<1
#define LIGHT    0x1<<2
#define SELECT   0x1<<3
EXPORT extern unsigned long draw_status;

EXPORT extern BWindow current_window;
EXPORT extern View current_view;
EXPORT extern int current_viewport[4];
EXPORT extern GLdouble raster_pos_select[4];
EXPORT extern GLdouble raster_pos[4];  // current raster position in windows coordinates
EXPORT extern int raster_hit;
EXPORT extern unsigned long light_index;
EXPORT extern const GLdouble identity_mat[16];
EXPORT extern const int trans[];
EXPORT extern const int source_mode[];
EXPORT extern const int dest_mode[];

EXPORT extern int pick_x;
EXPORT extern int pick_y;

EXPORT extern int pick_dx;
EXPORT extern int pick_dy;

EXPORT extern NativeType window_type;
EXPORT extern NativeType view_type;
EXPORT extern NativeType layer_type;
EXPORT extern NativeType shape_type;
EXPORT extern NativeType image_type;
EXPORT extern NativeType light_type;
EXPORT extern NativeType png_type;
EXPORT extern NativeType text_type;

EXPORT Link callback(Link parent, Link callback);

void events(void * args);


NATIVECALL(bwindow_update);
NATIVECALL(bwindow_setTitle);
NATIVECALL(bwindow_hide);
NATIVECALL(bwindow_show);
NATIVECALL(bwindow_setPosition);
NATIVECALL(bwindow_setSize);
void bwindow_free( BWindow win);
void bwindow_paint( BWindow win);

#endif

