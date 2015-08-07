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


static Display * display;
static XVisualInfo * vi;
static XSetWindowAttributes attributes;

static Atom bl_window;
static Atom bl_message;
static Atom wm_delete;
static Atom wm_protocols;

static int single_attributes[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None};

static int double_attributes[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None };

static const long event_mask=
            ExposureMask |
            KeyPressMask |
            KeyReleaseMask |
            ButtonPressMask |
            ButtonReleaseMask |
            PointerMotionMask |
            StructureNotifyMask;

void sendMessage(void * vp, unsigned long type , unsigned long message){
    BWindow self = vp;
    XEvent xe;
    xe.type = ClientMessage;
    xe.xclient.window = self->hwindow;
    xe.xclient.message_type = bl_message;
    xe.xclient.data.l[0] = type;
    xe.xclient.data.l[1] = (long)self;
    xe.xclient.data.l[2] = (long)message;
    XPutBackEvent(display, &xe);
    XSync(display, False);
}


void bwindow_paint( BWindow win){
    glXMakeCurrent(display, win->hwindow, win->context);
    bwindow_draw(win);
    glXSwapBuffers(display, win->hwindow);    
}

NATIVECALL(bwindow_update){
    BWindow self = This->value.vptr;
    self->update = 1;
    return link_dup(This);
}

NATIVECALL(bwindow_setTitle){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn < 1) {
        return exception("WindowsetTitleArgumentException", NULL, NULL);
    }

    win->title = object_asString(args[0]);
    XStoreName(display, win->hwindow,win->title->data);
    XSetIconName(display, win->hwindow,win->title->data);
    return link_dup(This);
}

NATIVECALL(bwindow_hide){
    BWindow win = This->value.vptr;
    XUnmapWindow(display, win->hwindow);
    return link_dup(This);
}

NATIVECALL(bwindow_show){
    BWindow win = This->value.vptr;
    XMapRaised(display, win->hwindow);
    XResizeWindow(display,win->hwindow,win->width,win->height);
    XMoveWindow(display,win->hwindow, win->x, win->y);
    return link_dup(This);
}

NATIVECALL(bwindow_setPosition){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    if (argn < 2) {
        return exception("WindowsetPositionArgumentException", NULL, NULL);
    }

    win->x  =  (int)object_asNumber(args[0]);
    win->y  =  (int)object_asNumber(args[1]);
    XMoveWindow(display,win->hwindow, win->x, win->y);
    return link_dup(This);
}



NATIVECALL(bwindow_setSize){
    BWindow win = This->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    if (argn < 2) {
        return exception("WindowsetSizeArgumentException", NULL, NULL);
    }
    win->width  =  (unsigned int)object_asNumber(args[0]);
    win->height =  (unsigned int)object_asNumber(args[1]);
    XResizeWindow(display,win->hwindow,win->width,win->height);    
    return link_dup(This);
}

void bwindow_free( BWindow win){
    //while(XCheckWindowEvent(display, win->hwindow, event_mask, &xevent)){}
    XDestroyWindow(display, win->hwindow);
    glXDestroyContext(display, win->context);
}




void bwindow_create(Link link){
    BWindow self = link->value.vptr;
    Window winDummy;
    unsigned int borderDummy;

       self->hwindow = XCreateWindow (
            display,
            DefaultRootWindow (display),
            self->x, self->y, self->width, self->height,
            0,
            vi->depth,
            InputOutput,
            vi->visual,
            CWColormap | CWEventMask,
            &attributes
            );

    //need this to make ClientNotify/WM_DELETE event fire
    XSetWMProtocols(display, self->hwindow, &wm_delete, 1);
    self->context = glXCreateContext(display, vi, 0, GL_TRUE);
    //tell the xwindow what blue-window it represents
    XChangeProperty(
        display,
        self->hwindow,
        bl_window,
        bl_window,
        8,
        PropModeReplace,
        (unsigned char *)&link,
        sizeof(BWindow) // sizeof struct BWindow *
        );

    XMapRaised(display, self->hwindow);
    XGetGeometry(display, self->hwindow, &winDummy, &self->x, &self->y,&self->width, &self->height, &borderDummy, &self->depth);
}





void events(void * args){
    
    static XEvent xevent;
    static Window xwin;
    static Window lastxwin = 0;
    static BWindow win     = NULL;
    static Link win_link   = NULL;
    static Link * link     = NULL;
    static Atom atom;
    static int format;
    static int status;
    static unsigned long nitems;
    static unsigned long remaining;
    static int event_ready=0;
    static int mousex=0;
    static int mousey=0;
    static int mousedx=0 ;
    static int mousedy=0;
    static Link ret = NULL;

        XNextEvent(display, &xevent);
        xwin = xevent.xany.window;

        /* if this window is the same as the last don't look it up */
        if (xwin != lastxwin) {
            mousex = 0;
            mousey = 0;
            status = XGetWindowProperty(
                display, // the display
                xwin, // the window
                bl_window, // name of property
                0,
                sizeof(Link)/4,
                False, // don't delete
                AnyPropertyType, //type
                &atom, // return type
                &format, // return format
                &nitems, // return number of items
                &remaining, // return bytes remaining
                (void *)&link);
                //(unsigned char **)&blwin);

            if (status == Success){
                lastxwin = xwin;
                win_link = *link;
                win = win_link->value.vptr;
                XFree(link);
            }else{
                return;
            }
        }

        switch (xevent.type){
            case ConfigureNotify:
                if (! win) break;
                win->width = xevent.xconfigure.width;
                win->height = xevent.xconfigure.height;
                win->x = xevent.xconfigure.x;
                win->y = xevent.xconfigure.y;
                break;
            case Expose:
                glXMakeCurrent(display, win->hwindow, win->context);
                bwindow_draw(win);
                glXSwapBuffers(display, win->hwindow);                
                break;
            case DestroyNotify:
                bwindow_free(win);
                break;

            /* EVENTS */
            case MotionNotify:
                win->event.state = BEVT_MOTION | BEVT_MOUSE;
                switch (xevent.xmotion.state & (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask) ){
                    case Button1Mask:
                        win->event.state |= BEVT_MOUSE1;
                        break;
                    case Button2Mask:
                        win->event.state |= BEVT_MOUSE2;
                        break;
                    case Button3Mask:
                        win->event.state |= BEVT_MOUSE3;
                        break;
                    case Button4Mask:
                        win->event.state |= BEVT_MOUSE4;
                        break;
                    case Button5Mask:
                        win->event.state |= BEVT_MOUSE5;
                        break;
                }

                while( XCheckTypedWindowEvent(display, xwin, MotionNotify,&xevent) ){};

                mousedx = xevent.xmotion.x - mousex;
                mousex = xevent.xmotion.x;

                mousedy = xevent.xmotion.y - mousey;
                mousey = xevent.xmotion.y;

                goto MaskCommon;
                break;

            case ButtonRelease:
                win->event.state = BEVT_RELEASE | BEVT_MOUSE;
                goto ButtonCommon;
                break;

            case ButtonPress:
                win->event.state = BEVT_PRESS | BEVT_MOUSE;
                goto ButtonCommon;
                break;

            case KeyRelease:
                if ( win->onKeyUp){
                    win->event.key = XLookupKeysym(&xevent.xkey, 0);
                    ret = object_call(win->onKeyUp,  win_link , NULL);
                    if (ret) link_free(ret);
                }
                break;

            case KeyPress:
                if ( win->onKeyDown){
                    win->event.key = XLookupKeysym(&xevent.xkey, 0);
                    ret = object_call(win->onKeyDown,  win_link , NULL);
                    if (ret) link_free(ret);
                }
                break;

            ButtonCommon:
                switch(xevent.xbutton.button){
                    case 1:
                        win->event.state |= BEVT_MOUSE1;
                        break;
                    case 2:
                        win->event.state |= BEVT_MOUSE2;
                        break;
                    case 3:
                        win->event.state |= BEVT_MOUSE3;
                        break;
                    case 4:
                        win->event.state |= BEVT_MOUSE4;
                        break;
                    case 5:
                        win->event.state |= BEVT_MOUSE5;
                        break;
                }

                win->event.key = 0;

                mousedx = xevent.xbutton.x - mousex;
                mousex = xevent.xbutton.x;

                mousedy = xevent.xbutton.y - mousey;
                mousey = xevent.xbutton.y;

                goto MaskCommon;
                break;


            MaskCommon:
                if (xevent.xbutton.state & ShiftMask)   win->event.state |= BEVT_SHIFT;
                if (xevent.xbutton.state & LockMask)    win->event.state |= BEVT_SHIFTLOCK;
                if (xevent.xbutton.state & ControlMask) win->event.state |= BEVT_CTRL;
                if (xevent.xbutton.state & Mod1Mask)    win->event.state |= BEVT_ALT;
                if (xevent.xbutton.state & Mod4Mask)    win->event.state |= BEVT_WIN;
                event_ready = 1;
                break;

            case ClientMessage:
                /* CLOSE WINDOW */
                    if (xevent.xclient.message_type == wm_protocols
                    && xevent.xclient.format == 32
                    && xevent.xclient.data.l[0] == wm_delete)
                    {
                        if ( win->onClose){
                            Link ret = object_call(win->onClose,  win_link , NULL);
                            link_free(ret);
                        }
                    }
                break;
        }

        if (event_ready){
            win->event.pos.x = mousex;
            win->event.dpos.x = mousedx;
            win->event.pos.y = mousey;
            win->event.dpos.y = mousedy;
            win->event.pos.z = 0;
            win->event.dpos.z = 0;
            bwindow_pick(win);
            event_ready = 0;
        }
        
        if ( win->update) bwindow_paint(win);        
}


void init(INITFUNC_ARGS){

    display = XOpenDisplay(0);

    vi = glXChooseVisual(display, DefaultScreen(display), double_attributes);
    if (! vi) vi = glXChooseVisual(display, DefaultScreen(display), single_attributes);

    bl_window    = XInternAtom (display, "bl_window", False);
    bl_message   = XInternAtom (display, "bl_message", False);
    wm_delete    = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);

    attributes.event_mask = event_mask;
    attributes.colormap   = XCreateColormap(display, RootWindow(display, vi->screen),vi->visual, AllocNone);

    blue_gl_init(lib, Module);    
}




