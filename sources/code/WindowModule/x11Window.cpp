#ifdef __linux__
#include "Input.h"
#include "Window.h"
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glx.h>

#include <cstring>

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef void *(APIENTRYP PFNGLCLEAR) (int n);
typedef void *(APIENTRYP PFNGLCLEARCOLOR) (double r, double g, double b, double a);

GameWindow::~GameWindow() {
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

static bool isExtensionSupported(const char *extList, const char *extension) {
    const char *start;
    const char *where, *terminator;

    where = strchr(extension, ' ');
    if (where || *extension == '\0') {
        return false;
    }

    for (start=extList;;) {
        where = strstr(start, extension);

        if (!where) {
            break;
        }

        terminator = where + strlen(extension);

        if ( where == start || *(where - 1) == ' ' ) {
            if ( *terminator == ' ' || *terminator == '\0' ) {
                return true;
            }
        }   

        start = terminator;
    }

    return false;
}

bool GameWindow::Initialize(const char *title, int resolutionX, int resolutionY) {
	
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		std::cout << "Could not open display\n";
		return false;
	}
	screen = DefaultScreenOfDisplay(display);
	screenID = DefaultScreen(display);

	GLint glxAttribs[] = {
		/*GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE,     24,
		GLX_STENCIL_SIZE,   8,
		GLX_RED_SIZE,       8,
		GLX_GREEN_SIZE,     8,
		GLX_BLUE_SIZE,      8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES,        0,
		None*/

      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      //GLX_SAMPLE_BUFFERS  , 1,
      //GLX_SAMPLES         , 4,
      None
	};

	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(display, screenID, glxAttribs, &fbcount);
	if (fbc == 0) {
		std::cout << "Failed to retrieve framebuffer.\n";
		XCloseDisplay(display);
		return 0;
	}
	//std::cout << "Found " << fbcount << " matching framebuffers.\n";

	// Pick the FB config/visual with the most samples per pixel
	//std::cout << "Getting best XVisualInfo\n";
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
		if ( vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );
			//std::cout << "  Matching fbconfig " << i << ", SAMPLE_BUFFERS = " << samp_buf << ", SAMPLES = " << samples << ".\n";

			if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) ) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree( vi );
	}

	//std::cout << "Best visual info index: " << best_fbc << "\n";
	GLXFBConfig bestFbc = fbc[ best_fbc ];
	XFree( fbc ); // Make sure to free this!

	XVisualInfo* visual = glXGetVisualFromFBConfig( display, bestFbc );

	if (visual == 0) {
		std::cout << "Could not create correct visual window.\n";
		XCloseDisplay(display);
		return 0;
	}

	if (screenID != visual->screen) {
		std::cout << "screenID(" << screenID << ") does not match visual->screen(" << visual->screen << ").\n";
		XCloseDisplay(display);
		return 0;
	}

	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(display, screenID);
	windowAttribs.background_pixel = WhitePixel(display, screenID);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenID), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask;

	window = XCreateWindow(display, RootWindow(display, screenID), 0, 0, 1024, 768, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Name the window
	XStoreName(display, window, "Named Window");

	// Create GLX OpenGL context
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,	3,
		GLX_CONTEXT_MINOR_VERSION_ARB,	3,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

    GLXContext context = 0;
    const char *glxExts = glXQueryExtensionsString( display,  screenID );
    if (!isExtensionSupported( glxExts, "GLX_ARB_create_context")) {
        std::cout << "GLX_ARB_create_context not supported\n";
        context = glXCreateNewContext( display, bestFbc, GLX_RGBA_TYPE, 0, True );
    }
    else {
        context = glXCreateContextAttribsARB( display, bestFbc, 0, true, context_attribs );
    }

	GLint majorGLX, minorGLX = 0;
	glXQueryVersion(display, &majorGLX, &minorGLX);
	if (majorGLX <= 1 && minorGLX < 2) {
		std::cout << "GLX 1.2 or greater is required.\n";
		XCloseDisplay(display);
		return false;
	}
	else {
		printf("GLX %i.%i supported.\n", majorGLX, minorGLX);
	}

	if (context == NULL) {
		std::cout << "Null Context!" << "\n";
		return 0;
	}


	XSync( display, False );

	// Verifying that context is a direct context
	if (!glXIsDirect (display, context)) {
		std::cout << "Indirect GLX rendering context obtained\n";
	}

	//if (!glXMakeCurrent(display, window, context))
	if (!glXMakeContextCurrent(display, window, window, context))
		std::cout << "GLX Make Current Failed!!" << "\n";

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

    /*XEvent ev;
	while (true) {
        XNextEvent(display, &ev);
        if (ev.type == Expose) {
            XWindowAttributes attribs;
            XGetWindowAttributes(display, window, &attribs);
            glViewport(0, 0, attribs.width, attribs.height);
        }

        // OpenGL Rendering
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
            glColor3f(  1.0f,  0.0f, 0.0f);
            glVertex3f( 0.0f, -1.0f, 0.0f);
            glColor3f(  0.0f,  1.0f, 0.0f);
            glVertex3f(-1.0f,  1.0f, 0.0f);
            glColor3f(  0.0f,  0.0f, 1.0f);
            glVertex3f( 1.0f,  1.0f, 0.0f);
        glEnd();

        // Present frame
        glXSwapBuffers(display, window);

	}*/

	// Resize window
	/*unsigned int change_values = CWWidth | CWHeight;
	XWindowChanges values;
	values.width = game.settings.resolution.x;
	values.height = game.settings.resolution.y;
	XConfigureWindow(display, window, change_values, &values);*/
	return true;
}

void GameWindow::SwapBuffer() {
        glXSwapBuffers(display, window);
}

void GameWindow::SetCursorShown(bool) {
	XDefineCursor(display, window, None);
}

void GameWindow::ResetCursor() {
	SetCursor(resX / 2, resY / 2);
}

void GameWindow::SetCursor(int x, int y) {
	XWarpPointer(display, None, window, 0, 0, 0, 0, x, y);
	XFlush(display); 
}

void GameWindow::GetCursor(int &x, int &y) {
	Window root, child;
	int rootX, rootY;
	unsigned int mask;
	XQueryPointer(display,window,&root,&child,&x,&y,&x,&y,&mask); 
}

int TranslateKey(int key) {
	switch (key) {
		case XK_A: case XK_a: return KEY_A;
		case XK_B: case XK_b: return KEY_B;
		case XK_C: case XK_c: return KEY_C;
		case XK_D: case XK_d: return KEY_D;
		case XK_E: case XK_e: return KEY_E;
		case XK_F: case XK_f: return KEY_F;
		case XK_G: case XK_g: return KEY_G;
		case XK_H: case XK_h: return KEY_H;
		case XK_I: case XK_i: return KEY_I;
		case XK_J: case XK_j: return KEY_J;
		case XK_K: case XK_k: return KEY_K;
		case XK_L: case XK_l: return KEY_L;
		case XK_M: case XK_m: return KEY_M;
		case XK_N: case XK_n: return KEY_N;
		case XK_O: case XK_o: return KEY_O;
		case XK_P: case XK_p: return KEY_P;
		case XK_Q: case XK_q: return KEY_Q;
		case XK_R: case XK_r: return KEY_R;
		case XK_S: case XK_s: return KEY_S;
		case XK_T: case XK_t: return KEY_T;
		case XK_U: case XK_u: return KEY_U;
		case XK_V: case XK_v: return KEY_V;
		case XK_W: case XK_w: return KEY_W;
		case XK_X: case XK_x: return KEY_X;
		case XK_Y: case XK_y: return KEY_Y;
		case XK_Z: case XK_z: return KEY_Z;
		case XK_Escape: return KEY_ESCAPE;
		case XK_Tab: return KEY_TAB;
		case XK_space: return KEY_SPACE;
		case XK_Control_L: return KEY_LCONTROL;
		case XK_Control_R: return KEY_CONTROL;
		case XK_Shift_L: return KEY_LSHIFT;
		case XK_Shift_R: return KEY_SHIFT;
		case XK_Alt_L: return KEY_LALT;
		case XK_Alt_R: return KEY_ALT;

		case XK_Left:	return KEY_LEFT;
		case XK_Right:	return KEY_RIGHT;
		case XK_Up:	return KEY_UP;
		case XK_Down:	return KEY_DOWN;

		case XK_KP_0: return KEY_NUMPAD_0;
		case XK_KP_1: return KEY_NUMPAD_1;
		case XK_KP_2: return KEY_NUMPAD_2;
		case XK_KP_3: return KEY_NUMPAD_3;
		case XK_KP_4: return KEY_NUMPAD_4;
		case XK_KP_5: return KEY_NUMPAD_5;
		case XK_KP_6: return KEY_NUMPAD_6;
		case XK_KP_7: return KEY_NUMPAD_7;
		case XK_KP_8: return KEY_NUMPAD_8;
		case XK_KP_9: return KEY_NUMPAD_9;

		case XK_Num_Lock:	return KEY_NUMPAD_NUMLOCK;
		case XK_KP_Divide: 	return KEY_NUMPAD_DIVIDE;
		case XK_KP_Multiply: 	return KEY_NUMPAD_MULTIPLY;
		case XK_KP_Subtract: 	return KEY_NUMPAD_SUBTRACT;
		case XK_KP_Add: 	return KEY_NUMPAD_ADD;
		case XK_KP_Enter: 	return KEY_NUMPAD_ENTER;
		case XK_KP_Decimal: 	return KEY_NUMPAD_DOT;

		case XK_F1: return KEY_F1;
		case XK_F2: return KEY_F2;
		case XK_F3: return KEY_F3;
		case XK_F4: return KEY_F4;
		case XK_F5: return KEY_F5;
		case XK_F6: return KEY_F6;
		case XK_F7: return KEY_F7;
		case XK_F8: return KEY_F8;
		case XK_F9: return KEY_F9;
		case XK_F10:return KEY_F10;
		case XK_F11:return KEY_F11;
		case XK_F12:return KEY_F12;
		case XK_F13:return KEY_F13;
		case XK_F14:return KEY_F14;
		case XK_F15:return KEY_F15;
		case XK_F16:return KEY_F16;
		case XK_F17:return KEY_F17;
		case XK_F18:return KEY_F18;
		case XK_F19:return KEY_F19;
		case XK_F20:return KEY_F20;
		case XK_F21:return KEY_F21;
		case XK_F22:return KEY_F22;
		case XK_F23:return KEY_F23;
		case XK_F24:return KEY_F24;
		case XK_F25:return KEY_F25;

		case XK_0:	return KEY_0;
		case XK_1:	return KEY_1;
		case XK_2:	return KEY_2;
		case XK_3:	return KEY_3;
		case XK_4:	return KEY_4;
		case XK_5:	return KEY_5;
		case XK_6:	return KEY_6;
		case XK_7:	return KEY_7;
		case XK_8:	return KEY_8;
		case XK_9:	return KEY_9;
		case XK_minus:	return KEY_DASH;
		case XK_equal:	return KEY_ADD;

		case XK_Insert:		return KEY_INSERT;
		case XK_Home:		return KEY_HOME;
		case XK_Page_Up:	return KEY_PG_UP;
		case XK_Page_Down:	return KEY_PG_DOWN;
		case XK_End:		return KEY_END;
		case XK_KP_Delete:	return KEY_DELETE;
		case XK_Pause:		return KEY_PAUSE;
		case XK_Caps_Lock:	return KEY_CAPSLOCK;
		case XK_Scroll_Lock:	return KEY_SCROLL_LOCK;

		case XK_comma:		return KEY_COMMA;
		case XK_period:		return KEY_PERIOD;
		case XK_slash:		return KEY_FORWARD_SLASH;
		case XK_backslash:	return KEY_BACK_SLASH;
		case XK_semicolon:	return KEY_SEMICOLON;
		case XK_apostrophe:	return KEY_APOSTROPHE;
		case XK_bracketleft:	return KEY_LBRACKET;
		case XK_bracketright:	return KEY_RBRACKET;

		case XK_Return:		return KEY_ENTER;
		case XK_BackSpace:	return KEY_BACKSPACE;
		case XK_grave:		return KEY_TILDE;
		default: return -1;
	}
}

void GameWindow::HandleEvents() {
	if (input == NULL) {
		std::cout << "No Interface!" << std::endl;
		return;
	}
	XWindowAttributes attribs;
	char str[25] = {0}; 
	KeySym keysym = 0;
	int len = 0;
	XEvent ev;
        while (XPending(display) > 0) {
		XNextEvent(display, &ev);
		switch(ev.type) {
			case KeymapNotify:
				XRefreshKeyboardMapping(&ev.xmapping);
				break;
			case KeyPress:
				len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
				input->SetKey(TranslateKey(keysym), true);
				break;
			case KeyRelease:
				len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
				input->SetKey(TranslateKey(keysym), false);
				break;
			case ButtonPress:
				input->SetMouseButton(ev.xbutton.button-1, true);
	   			break;
			case ButtonRelease:
				input->SetMouseButton(ev.xbutton.button-1, false);
				break;
			case MotionNotify:
				input->SetMousePosition(ev.xmotion.x, ev.xmotion.y);
				break;
           	case FocusIn:
				input->SetFocused(true);
				break;
           	case FocusOut:
				input->SetFocused(false);
				break;
			case DestroyNotify:
				input->ForceQuit();
				break;
			/*case Expose: {
				//XWindowAttributes attribs;
				std::cout << "Expose event fired: " << "\n";
				XGetWindowAttributes(display, window, &attribs);
				//std::cout << "\tWindow Width: " << attribs.width << ", Height: " << attribs.height << "\n";
				}
				break;*/
		}
	}
}

void GameWindow::GetHandles(Display* dpy, Window *win, Screen* scrn, int id) {
		Display* display;
		Window window;
		Screen *screen;
		id = screenID;
}

void GameWindow::Shutdown() {
	delete this;
}

#endif