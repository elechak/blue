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
#include <windows.h>        // Header File For Windows
#include <windowsx.h>        // Header File For Windows

extern int __argc;
extern char ** __argv;


//struct Event event;
int lastx, lasty;
int button[8][3];
float scalex,scaley;
int output_descriptor;
char output_type;
HDC            hDC=NULL;        // Private GDI Device Context
HGLRC        hRC=NULL;        // Permanent Rendering Context  // win->context
HWND        hWnd=NULL;        // Holds Our Window Handle  // win->hwindow
HINSTANCE    hInstance;        // Holds The Instance Of The Application
BOOL    keys[256];            // Array Used For The Keyboard Routine
BOOL    active=TRUE;        // Window Active Flag Set To TRUE By Default
DWORD graphics_thread;
DWORD        default_style;                // Window Style
WNDCLASS    wc;                        // Windows Class Structure


static    PIXELFORMATDESCRIPTOR pfd={
    sizeof(PIXELFORMATDESCRIPTOR),        // Size Of This Pixel Format Descriptor
    1,                                                     // Version Number
    PFD_DRAW_TO_WINDOW |                // Format Must Support Window
    PFD_SUPPORT_OPENGL |                    // Format Must Support OpenGL
    PFD_DOUBLEBUFFER,                        // Must Support Double Buffering
    PFD_TYPE_RGBA,                                // Request An RGBA Format
    0,                                                  // Select Our Color Depth
    0, 0, 0, 0, 0, 0,                                // Color Bits Ignored
    0,                                                    // No Alpha Buffer
    0,                                                    // Shift Bit Ignored
    0,                                                    // No Accumulation Buffer
    0, 0, 0, 0,                                         // Accumulation Bits Ignored
    16,                                                // 16Bit Z-Buffer (Depth Buffer)
    0,                                                    // No Stencil Buffer
    0,                                                    // No Auxiliary Buffer
    PFD_MAIN_PLANE,                            // Main Drawing Layer
    0,                                                   // Reserved
    0, 0, 0                                           // Layer Masks Ignored
};




LRESULT    CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);    // Declaration For WndProc



void sendMessage(void * vp, unsigned long type ,unsigned long message){
    PostThreadMessage(graphics_thread, type,(WPARAM)vp, message);
}




static void bwindow_paint(BWindow self){
    wglMakeCurrent( GetDC(self->hwindow) ,self->context);
    bwindow_draw(self);
    SwapBuffers(hDC);
}




void bwindow_create_win(Link link){
    BWindow self = link->value.vptr;
    //char * title = "";

    GLuint        PixelFormat;            // Holds The Results After Searching For A Match

    DWORD        dwExStyle;                // Window Extended Style
    RECT        WindowRect;                // Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left=(long)0;            // Set Left Value To 0
    WindowRect.right=(long)self->width;        // Set Right Value To Requested Width
    WindowRect.top=(long)0;                // Set Top Value To 0
    WindowRect.bottom=(long)self->height;        // Set Bottom Value To Requested Height

    hInstance          = GetModuleHandle(NULL);                // Grab An Instance For Our Window

    dwExStyle=WS_EX_APPWINDOW;            // Window Extended Style
    default_style=WS_OVERLAPPEDWINDOW;                            // Windows Style
    // Create The Window
    self->hwindow=CreateWindowEx(
            dwExStyle,                              // Extended Style For The Window
            "blue",                                  // Class Name
            "",                                       // Window Title
            default_style,                          // Defined Window Style
            0, 0,                                    // Window Position
            self->width,                            // Width
            self->height,                            // Height
            NULL,                                   // No Parent Window
            NULL,                                   // No Menu
            hInstance,                            // Instance
            NULL);

    SetProp(self->hwindow, "bl_window", (HANDLE)link);
    hDC=GetDC(self->hwindow)  ;                       //  Device Context
    PixelFormat=ChoosePixelFormat(hDC,&pfd);       // Pixel Format
    SetPixelFormat(hDC,PixelFormat,&pfd)    ;           // Set Pixel Format
    self->context=wglCreateContext(hDC);            // Rendering Context

    ShowWindow(self->hwindow,SW_SHOW);        // Show The Window
    SetWindowPos(self->hwindow, HWND_NOTOPMOST,self->x,self->y,self->width,self->height,0);
    SetForegroundWindow(self->hwindow);           // Slightly Higher Priority
    SetFocus(self->hwindow);                           // Sets Keyboard Focus To The Window
}



void bwindow_create(Link link){
    sendMessage(link, CREATE, 0);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT  uMsg,  WPARAM  wParam, LPARAM  lParam){
    Binding binding;
    RECT rect;

    Link win_link = (Link)GetProp(hWnd, "bl_window");
    if (! win_link){
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }

    BWindow win = win_link->value.vptr;
    if (! win){
        //printf("NOWIN\n");
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }

    //printf("in windproc got %i\n", uMsg);fflush(stdout);

    switch (uMsg){

        case WM_PAINT:
            //printf("PAINT\n");
            bwindow_paint(win);
            break;

        case WM_SIZE:
            //printf("SIZE\n");
            GetClientRect(hWnd, &rect);
            win->width = rect.right-rect.left;
            win->height= rect.bottom-rect.top;
            break;

        case WM_CLOSE:
            //DPRINT("WMCLOSE");
            break;

        case WM_DESTROY:
            bwindow_free(win);
            //printf("DESTROY\n");
            break;

        case WM_SYSCOMMAND:
            switch (wParam){
                case SC_CLOSE:
                    //printf("SC_CLOSE\n");
                    ShowWindow(win->hwindow,SW_HIDE);        // Show The Window
                    win->event.state = BEVT_CLOSE;
                    binding = win->bindings;
                    while(binding){
                        if( (binding->event.state & win->event.state) == binding->event.state){
                            callback(win_link,binding->callback);
                        }
                        binding = binding->next;
                    }
                    return 0;
            }
            break;

    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


void event_loop(void * args){

    MSG        msg;
    Link win_link;
    BWindow win;
    int event_ready=0;

    static unsigned char keystate[256];

    int mousex , mousey, mousedx, mousedy;
    mousex= mousey= mousedx= mousedy = 0;

    // WINSPEC
    while (GetMessage(&msg,NULL,0,0)){

        if (msg.message == CREATE){
                bwindow_create_win((Link) msg.wParam);
                continue;
        }else if ((msg.message>=UPDATE) &&(msg.message < CREATE)){
            win = (BWindow) msg.wParam;
            switch (msg.message){
                    case UPDATE:
                        bwindow_paint(win);
                        break;
                    case RESIZE:
                        SetWindowPos(win->hwindow, HWND_NOTOPMOST,win->x,win->y,win->width,win->height,0);
                        break;
                    case POSITION:
                        SetWindowPos(win->hwindow, HWND_NOTOPMOST,win->x,win->y,win->width,win->height,0);
                        break;
                    case TITLE:
                        SetWindowText(win->hwindow,win->title->data);
                        break;
                    case VISIBILITY:
                        if (msg.lParam == 0)
                            ShowWindow(win->hwindow,SW_HIDE);
                        else
                            ShowWindow(win->hwindow,SW_SHOW);
                        break;
                    case FREE:        // free message
                        wglDeleteContext(win->context);
                        //ReleaseDC(win->hwindow,hDC);
                        DestroyWindow(win->hwindow);
                        //UnregisterClass("Bat",hInstance);
                        break;      // Jump Back
            }

        }else {
            win_link = (Link)GetProp(msg.hwnd, "bl_window");
            if (! win_link) goto default_dispatch;

            win = win_link->value.vptr;
            if (! win) goto default_dispatch;

            switch (msg.message){

                case WM_MOUSEMOVE:

                    win->event.state = BEVT_MOTION | BEVT_MOUSE;

                    if (msg.wParam & MK_LBUTTON) win->event.state |= BEVT_MOUSE1;
                    else if (msg.wParam & MK_MBUTTON) win->event.state |= BEVT_MOUSE2;
                    else if (msg.wParam & MK_RBUTTON) win->event.state |= BEVT_MOUSE3;

                    mousedx = GET_X_LPARAM(msg.lParam) - mousex;
                    mousex = GET_X_LPARAM(msg.lParam);

                    mousedy = GET_Y_LPARAM(msg.lParam) - mousey;
                    mousey = GET_Y_LPARAM(msg.lParam);


                    goto MaskCommon;
                    break;


                /* BUTTON RELEASE  */
                case WM_LBUTTONUP:
                    win->event.state = BEVT_RELEASE | BEVT_MOUSE | BEVT_MOUSE1;
                    goto ButtonCommon;
                    break;

                case WM_MBUTTONUP:
                    win->event.state = BEVT_RELEASE | BEVT_MOUSE | BEVT_MOUSE2;
                    goto ButtonCommon;
                    break;

                case WM_RBUTTONUP:
                    win->event.state = BEVT_RELEASE | BEVT_MOUSE | BEVT_MOUSE3;
                    goto ButtonCommon;
                    break;


                /* BUTTON PRESS */
                case WM_LBUTTONDOWN:
                    win->event.state = BEVT_PRESS | BEVT_MOUSE |BEVT_MOUSE1 ;
                    goto ButtonCommon;
                    break;
                case WM_MBUTTONDOWN:
                    win->event.state = BEVT_PRESS | BEVT_MOUSE |BEVT_MOUSE2 ;
                    goto ButtonCommon;
                    break;
                case WM_RBUTTONDOWN:
                    win->event.state = BEVT_PRESS | BEVT_MOUSE |BEVT_MOUSE3 ;
                    goto ButtonCommon;
                    break;


                /* KEY PRESS */
                case WM_KEYUP:
                    win->event.state = BEVT_RELEASE | BEVT_KEY;
                    goto KeyCommon;
                    break;

                /* KEY RELEASE */
                case WM_KEYDOWN:
                    win->event.state = BEVT_PRESS | BEVT_KEY;


                KeyCommon:
                    mousedx = GET_X_LPARAM(msg.lParam) - mousex;
                    mousex = GET_X_LPARAM(msg.lParam);

                    mousedy = GET_Y_LPARAM(msg.lParam) - mousey;
                    mousey = GET_Y_LPARAM(msg.lParam);

                    win->event.key = (unsigned long) msg.wParam;
                    goto MaskCommon;
                    break;


                ButtonCommon:
                    win->event.key = 0;

                    mousedx = GET_X_LPARAM(msg.lParam) - mousex;
                    mousex = GET_X_LPARAM(msg.lParam);

                    mousedy = GET_Y_LPARAM(msg.lParam) - mousey;
                    mousey = GET_Y_LPARAM(msg.lParam);

                    goto MaskCommon;
                    break;


                MaskCommon:
                    GetKeyboardState(keystate);
                    if (keystate[VK_SHIFT] & 0x80)      win->event.state |= BEVT_SHIFT;
                    if (keystate[20] & 0x01)                win->event.state |= BEVT_SHIFTLOCK;
                    if (keystate[VK_CONTROL] & 0x80)  win->event.state |= BEVT_CTRL;
                    if (keystate[VK_MENU] & 0x80)       win->event.state |= BEVT_ALT;
                    if (keystate[VK_NUMLOCK] & 0x01)  win->event.state |= BEVT_NUMLOCK;
                    event_ready = 1;
                    break;
            }
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
            continue;
        }


        default_dispatch:
        TranslateMessage(&msg);// Translate The Message
        DispatchMessage(&msg);// Dispatch The Message
    }
}



void init(INITFUNC_ARGS){
    blue_gl_init(lib, Module);

    CreateThread(NULL,0,(void *) event_loop,NULL,0,&graphics_thread);

    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;    // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc  = (WNDPROC) WndProc;                    // WndProc Handles Messages
    wc.cbClsExtra     = 0;                                    // No Extra Window Data
    wc.cbWndExtra   = 0;                                    // No Extra Window Data
    wc.hInstance      = hInstance;                            // Set The Instance
    wc.hIcon            = LoadIcon(NULL, IDI_WINLOGO);            // Load The Default Icon
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);            // Load The Arrow Pointer
    wc.hbrBackground= NULL;                                    // No Background Required For GL
    wc.lpszMenuName= NULL;                                    // We Don't Want A Menu
    wc.lpszClassName= "blue";                                // Set The Class Name
    RegisterClass(&wc);

    Sleep(0);
    Sleep(0);
    Sleep(0);
    Sleep(0);
    Sleep(0);
}


