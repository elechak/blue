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

static void bwindow_paint(BWindow self){
    glXMakeCurrent(display, self->hwindow, self->context);
    bwindow_draw(self);
    glXSwapBuffers(display, self->hwindow);
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



void graphics_at_exit(){
    XCloseDisplay(display);
}



void event_loop(void * args){
    static XEvent xevent;
    Binding binding;
    Window xwin;
    //Context context;
    Window lastxwin = 0;
    BWindow win = NULL;
    Link * link = NULL;
    Link win_link = NULL;
    Atom atom;
    int format;
    int status;
    unsigned long nitems;
    unsigned long remaining;
    int event_ready=0;
    int mousex , mousey, mousedx, mousedy;

    mousex= mousey= mousedx= mousedy = 0;

    //atexit(graphics_at_exit);

    while(1){
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
                continue;
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
                bwindow_paint(win);
                break;
            case DestroyNotify:
                //while(XCheckWindowEvent(display, win->hwindow, event_mask, &xevent)){}
                //glXMakeCurrent(display, None, NULL);
                //glXDestroyContext(display, win->context);
                //DPRINT("DESTROY MESSAGE")
                bwindow_free(win);
                break;

            /* EVENTS */
            case MotionNotify:
                win->event.state = BEVT_MOTION | BEVT_MOUSE;
                //printf("Motion %i  %i  %i\n", xevent.xmotion.state,xevent.xmotion.x,xevent.xmotion.y);
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
                //printf("Button Release %i  %i  %i\n", xevent.xbutton.button,xevent.xbutton.x,xevent.xbutton.y);
                goto ButtonCommon;
                break;

            case ButtonPress:
                win->event.state = BEVT_PRESS | BEVT_MOUSE;
                //printf("Button Press %i  %i  %i\n", xevent.xbutton.button,xevent.xbutton.x,xevent.xbutton.y);
                goto ButtonCommon;
                break;

            case KeyRelease:
                win->event.state = BEVT_RELEASE | BEVT_KEY;
                //printf("Key Release %i  %i  %i\n", (int)XLookupKeysym(&xevent.xkey, 0),xevent.xkey.x,xevent.xkey.y);
                goto KeyCommon;
                break;

            case KeyPress:
                win->event.state = BEVT_PRESS | BEVT_KEY;
                //printf("Key Press %i  %i  %i\n", (int)XLookupKeysym(&xevent.xkey, 0),xevent.xkey.x,xevent.xkey.y);
                goto KeyCommon;
                break;

            KeyCommon:
                mousedx = xevent.xkey.x - mousex;
                mousex = xevent.xkey.x;

                mousedy = xevent.xkey.y - mousey;
                mousey = xevent.xkey.y;

                win->event.key = XLookupKeysym(&xevent.xkey, 0);
                goto MaskCommon;
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

                        XUnmapWindow(display, win->hwindow);

                        win->event.state = BEVT_CLOSE;
                        binding = win->bindings;
                        while(binding){
                            if( (binding->event.state & win->event.state) == binding->event.state){
                                callback(win_link,binding->callback);
                            }
                            binding = binding->next;
                        }

                    }else if (xevent.xclient.message_type == bl_message){
                        win = (BWindow) xevent.xclient.data.l[1];

                        switch (xevent.xclient.data.l[0]){
                /* UPDATE */
                            case UPDATE:
                                bwindow_paint(win);
                                break;
                /* RESIZE */
                            case RESIZE:
                                XResizeWindow(display,win->hwindow,win->width,win->height);
                                break;
                /* POSITION */
                            case POSITION:
                                XMoveWindow(display,win->hwindow, win->x, win->y);
                                break;
                /* TITLE */
                            case TITLE:
                                XStoreName(display, win->hwindow,win->title->data);
                                XSetIconName(display, win->hwindow,win->title->data);
                                break;

                /* VISIBILITY */
                            case VISIBILITY:
                                if (xevent.xclient.data.l[2] == 0 ){
                                    XUnmapWindow(display, win->hwindow);
                                }else{
                                    XMapRaised(display, win->hwindow);
                                    XResizeWindow(display,win->hwindow,win->width,win->height);
                                    XMoveWindow(display,win->hwindow, win->x, win->y);
                                }
                                break;
                /* FREE */
                            case FREE:
                                    //DPRINT("FREE MESSAGE")
                                    //glFlush();
                                    while(XCheckWindowEvent(display, win->hwindow, event_mask, &xevent)){}
                                    //glXMakeCurrent(display, None, NULL);
                                    //context = win->context;
                                    XDestroyWindow(display, win->hwindow);
                                    glXDestroyContext(display, win->context);
                                    //glXMakeCurrent(display, None, NULL);
                                    //XUnmapWindow(display, win->hwindow);
                                    //DPRINT("FREE MESSAGE DONE")
                                    break;
                        }
                    }
                break;
        }

        if (event_ready){
            //printf("WINDOW %g %g\n", win->event.pos.x,win->event.pos.y);

            win->event.pos.x = mousex;
            win->event.dpos.x = mousedx;

            win->event.pos.y = mousey;
            win->event.dpos.y = mousedy;

            win->event.pos.z = 0;
            win->event.dpos.z = 0;

            bwindow_pick(win);
            event_ready = 0;
        }
    }
}








void init(INITFUNC_ARGS){
    blue_gl_init(lib, Module);

    /* Set up the X environment */
    XInitThreads();
    display = XOpenDisplay(0);

    vi = glXChooseVisual(display, DefaultScreen(display), double_attributes);
    if (! vi) vi = glXChooseVisual(display, DefaultScreen(display), single_attributes);

    bl_window    = XInternAtom (display, "bl_window", False);
    bl_message   = XInternAtom (display, "bl_message", False);
    wm_delete    = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);

    attributes.event_mask = event_mask;
    attributes.colormap   = XCreateColormap(display, RootWindow(display, vi->screen),vi->visual, AllocNone);

    /* start the event loop */
    run_async(event_loop, NULL);
}




