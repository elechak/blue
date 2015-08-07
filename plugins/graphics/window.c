

#include "graphics.h"

/* COMMON */
unsigned long draw_status = 0;
unsigned long light_index = 0;
GLdouble raster_pos_select[4];
GLdouble raster_pos[4];
int raster_hit;
int current_viewport[4];
NativeType window_type = NULL;
NativeType view_type = NULL;
NativeType layer_type = NULL;
NativeType shape_type = NULL;
NativeType image_type = NULL;
NativeType light_type = NULL;

BWindow current_window = NULL;
View    current_view   = NULL;

const GLdouble identity_mat[16] =  {1.0 ,0.0 ,0.0 ,0.0,
                               0.0 ,1.0 ,0.0 ,0.0 ,
                               0.0 ,0.0 ,1.0 ,0.0 ,
                               0.0 ,0.0 ,0.0 ,1.0 };

const int trans[]={ GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4,
                       GL_LIGHT5, GL_LIGHT6, GL_LIGHT7 };

const int source_mode[] ={ GL_ZERO, GL_ONE, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
    GL_SRC_ALPHA_SATURATE};

const int dest_mode[] ={GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA};

int pick_x =0;
int pick_y =0;
    
int pick_dx =0;
int pick_dy =0;


/* initialize blue and opengl stuff */
void blue_gl_init(INITFUNC_ARGS){
    
    //Global->threads = 1;
    //threads_enable();
    
    link_setThreadState(1);
    /* Blue type initialization */
    window_type = window_init(lib, Module);
    view_type   = view_init(lib, Module);
    layer_type  = layer_init(lib, Module);
    shape_type  = shape_init(lib, Module);
    image_type  = image_init(lib, Module);
    light_type  = light_init(lib, Module);

    //~ /* OpenGL initialization */
    //~ glEnable(GL_DEPTH_TEST); //enables hidden surface removal
    //~ glEnable(GL_NORMALIZE);  // slower but makes life a lot easier
    //~ glEnable(GL_TEXTURE_2D);       /* Enable Texture Mapping */

    //~ glDepthFunc(GL_LEQUAL);
    //~ glClearDepth(1.0);      // value used when clearing depth buffer
    //~ glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
    //~ glShadeModel(GL_SMOOTH);
    //~ glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //~ glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
}


void forceHit(){
    static GLdouble point[4];
    
    point[0] = pick_x;
    point[1] = pick_y;
    
    glmat_win2obj(point);
    
    glBegin(GL_POINTS);
        glVertex3dv(point);//draw a single point to force a hit
    glEnd();
}


void bwindow_draw(BWindow self){

    if (self != current_window){
        current_window = self;
        
        glEnable(GL_DEPTH_TEST); //enables hidden surface removal
        glEnable(GL_NORMALIZE);  // slower but makes life a lot easier
        glEnable(GL_TEXTURE_2D);       /* Enable Texture Mapping */

        glDepthFunc(GL_LEQUAL);
        glClearDepth(1.0);      // value used when clearing depth buffer
        //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        glShadeModel(GL_SMOOTH);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    }

    glClearColor( self->clear_color[0], self->clear_color[1], self->clear_color[2], self->clear_color[3]);
    draw_status = 0;
    glViewport(0,0,self->width,self->height);
    current_viewport[0] = 0;
    current_viewport[1] = 0;
    current_viewport[2] = self->width;
    current_viewport[3] = self->height;

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawChildren((ChildMan) self);
}


// Select
void bwindow_pick(BWindow self){
    static GLuint selectionBuffer[256];

    GLuint *ptr, *selection, num_obj, z1,z2,z;
    GLint hits;
    int num, i,j;

    raster_hit =0;
    current_window = self;
    draw_status = SELECT;

    self->event.pos.y = self->height - self->event.pos.y;
    self->event.dpos.y *=-1;

    pick_dx = self->event.pos.x - pick_x;
    pick_dy = self->event.pos.y - pick_y;
    
    pick_x = self->event.pos.x;
    pick_y = self->event.pos.y;
    
    

    //printf("DPOS %f %f\n", self->event.dpos.x, self->event.dpos.y);

    // initialize selection buffer
    glSelectBuffer(256,selectionBuffer);

    //--------------------------------------------------------------------------------------------
    //glEnable(GL_DEPTH_TEST); //enables hidden surface removal
    //glDepthFunc(GL_LEQUAL);
    glViewport(0,0,self->width,self->height);
    current_viewport[0] = 0;
    current_viewport[1] = 0;
    current_viewport[2] = self->width;
    current_viewport[3] = self->height;

    //glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClearDepth(1.0);
    //glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //--------------------------------------------------------------------------------------------

    /*
        * Hit stack looks like this:
        *
        * num_obj (GLuint)- number of objects on stack
        * z1 (GLuint) - value of closest point in hit
        * z2 (GLuint) - value of farthest point in hit
        * first object in hit  - this is the id (or in this case pointer) of the first object in hit
        *  .
        *  .
        * n  object in hit  - this is the id (or in this case pointer) of the n object in hit
        */

    ptr = selectionBuffer;
    num_obj = z1 = z2 = z=0;
    selection=NULL;

    glRenderMode(GL_SELECT);
    glInitNames();

    // iterate through views and draw them
    drawChildren((ChildMan) self);

    hits = glRenderMode(GL_RENDER); //number of hit stacks

    //printf("Hits %i\n", hits);

    if (hits){

        // initialize values for first hit
        num_obj = num = *ptr++;  // number of objects in stack
        z = z1 =  *ptr++;  // min depth of hit
        z2 =  *ptr++;  // max depth of hit
        selection = ptr; // selection remembers closest hit

        // if there is more than one hit selection will be the one with the nearest point
        for (i = 1; i<hits ; i++){
            ptr+=num; // increment ahead to next hit
            num = *ptr++;
            z1 =  *ptr++;
            z2 =  *ptr++;

            if (z1 < z){
                selection = ptr;
                z = z1;
                num_obj=num;
            }
        }

        // num is the number of hits in this stack
        // j = 0 is the view
        Layer lay  = NULL;
        Link child = NULL;
        Binding binding = NULL;

        view_transform(current_view);
        for (j = 0 ; j< num_obj ; j++){
            child=(Link) index_get(bound_layers, *selection++);
            lay = child->value.vptr;
            layer_transform(child->value.vptr);
            
            binding = lay->bindings;
            while(binding){
                if(     ((binding->event.state & current_window->event.state) == binding->event.state) &&
                        (binding->event.key == current_window->event.key))  
                {
                    callback(child,binding->callback);
                }

                binding = binding->next;
            }
        }
    }
}



static NATIVECALL(bwindow_new){
    BWindow self = malloc(sizeof(*self));

        self->children  = list_new(sizeof(ListNode));

        self->width      = 500;
        self->height     = 500;
        self->title      = NULL;
        self->x = self->y = 0;

        self->clear_color[0] = 1;
        self->clear_color[1] = 1;
        self->clear_color[2] = 1;
        self->clear_color[3] = 1;

        self->bindings=NULL;

    Link obj = object_create(window_type);
    obj->value.vptr = self;

    //DPRINT("window link is %p", obj)

    bwindow_create(obj);

    return obj;
}

static void destroy(Link self){
    BWindow win = self->value.vptr;
    sendMessage(win, FREE , 0);

    if (win->children)    freeChildren((ChildMan)win);
    //DPRINT("DONE WITH DESTROY")
    object_destroy(self);
}

// This is called by the os/gui dependant file graphics_*
void bwindow_free(BWindow self){
    //DPRINT("FREE")
    free(self);
}

static NATIVECALL(bwindow_setColor){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    size_t count;
    
    if (argn > 3) argn=3;
    for ( count = 0; count < argn; count++){
        win->clear_color[count] = object_asNumber(args[count]);
    }
    win->clear_color[3] = 1.0;
    return link_dup(This);
}

static NATIVECALL(bwindow_update){
    sendMessage(This->value.vptr, UPDATE , 0);
    return object_create(Global->null_type);
}

static NATIVECALL(bwindow_setSize){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn < 2) {
        return exception("WindowsetSizeArgumentException", NULL, NULL);
    }

    win->width  =  (unsigned int)object_asNumber(args[0]);
    win->height =  (unsigned int)object_asNumber(args[1]);
    sendMessage(win, RESIZE , 0);
    return link_dup(This);
}

static NATIVECALL(bwindow_setPosition){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn < 2) {
        return exception("WindowsetPositionArgumentException", NULL, NULL);
    }

    win->x  =  (int)object_asNumber(args[0]);
    win->y  =  (int)object_asNumber(args[1]);
    sendMessage(win, POSITION , 0);
    return link_dup(This);
}

static NATIVECALL(bwindow_setTitle){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn < 1) {
        return exception("WindowsetTitleArgumentException", NULL, NULL);
    }

    win->title = object_asString(args[0]);
    sendMessage(win, TITLE , 0);
    return link_dup(This);
}

static NATIVECALL(bwindow_hide){
    BWindow win = This->value.vptr;
    sendMessage(win, VISIBILITY , 0);
    return link_dup(This);
}

static NATIVECALL(bwindow_show){
    BWindow win = This->value.vptr;
    sendMessage(win, VISIBILITY , 1);
    return link_dup(This);
}

static NATIVECALL(bwindow_bind){
    BWindow self   = This->value.vptr;
    Link * args  = array_getArray(Arg);

    string_t string_event = object_getString( args[0] );
    Binding binding = binding_new(string_event->data, link_dup(args[1]));

    binding->next = self->bindings;
    self->bindings = binding;

    return link_dup(This);
}

NativeType window_init(INITFUNC_ARGS){
    NativeType t = newNative();
    child_manager(t);
    t->lib          = lib;
    t->destroy   = destroy;
    
    addNativeCall(t, "update", bwindow_update);
    addNativeCall(t, "setColor", bwindow_setColor);
    addNativeCall(t, "setSize", bwindow_setSize);
    addNativeCall(t, "setPosition", bwindow_setPosition);
    addNativeCall(t, "setTitle", bwindow_setTitle);
    addNativeCall(t, "hide", bwindow_hide);
    addNativeCall(t, "show", bwindow_show);
    addNativeCall(t, "bind", bwindow_bind);

    addCFunc(Module, "window", bwindow_new);
    
    
    return t;
}


Link callback(Link parent, Link callback){
    /* call the function */
    Link ret = object_call(callback,  parent,NULL);
    return ret;
}



