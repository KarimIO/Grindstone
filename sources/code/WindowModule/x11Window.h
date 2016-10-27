#ifdef __linux__
#ifndef _WINDOW_H
#define _WINDOW_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glx.h>
#include <stdio.h>

int launchWindow() {	
	Display* display;
    Window window;
    XEvent event;
    char* message = "Hello";
    int screenSize;

    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    screenSize = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screenSize), 10, 10, 1920, 1080, 1, BlackPixel(display, screenSize), WhitePixel(display, screenSize));
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
	XFlush(display);
	
    KeySym keysym = XK_Escape;
    KeyCode keycode = XKeysymToKeycode(display, keysym);

    while(true)
    {
        XNextEvent(display, &event);
        if(event.type == KeyPress && event.xkey.keycode == keycode)
            break;
    }

    XCloseDisplay(display);
	
	return true;
}


#endif
#endif