#ifdef __linux__
#include "Window.h"
#include <stdio.h>

GameWindow::~GameWindow() {
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

bool GameWindow::Initialize() {	
    XEvent event;
    char* message = "Hello";
    int screenSize;

    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

	screen = DefaultScreenOfDisplay(display);
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