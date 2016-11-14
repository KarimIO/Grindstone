//#ifdef __linux__
#include "OpenGLGraphics.h"

bool GraphicsWrapper::SetWindowContext() { // Check GLX version
	int majorGLX, minorGLX = 0;
	glXQueryVersion(display, &majorGLX, &minorGLX);
	if (majorGLX <= 1 && minorGLX < 2) {
		std::cout << "GLX 1.2 or greater is required.\n";
		XCloseDisplay(display);
		return false;
	}

	GLint glxAttribs[] = {
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
		None
	};

	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(display, screenId, glxAttribs, &fbcount);
	if (fbc == 0) {
		std::cout << "Failed to retrieve framebuffer.\n";
		XCloseDisplay(display);
		return 0;
	}
	//std::cout << "Found " << fbcount << " matching framebuffers.\n";

	// Pick the FB config/visual with the most samples per pixel
	//std::cout << "Getting best XVisualInfo\n";
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = INT_MAXS;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
		if (vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);
			//std::cout << "  Matching fbconfig " << i << ", SAMPLE_BUFFERS = " << samp_buf << ", SAMPLES = " << samples << ".\n";

			if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree(vi);
	}

	//std::cout << "Best visual info index: " << best_fbc << "\n";
	GLXFBConfig bestFbc = fbc[best_fbc];
	XFree(fbc); // Make sure to free this!

	XVisualInfo* visual = glXGetVisualFromFBConfig(display, bestFbc);

	if (visual == 0) {
		std::cout << "Could not create correct visual window.\n";
		XCloseDisplay(display);
		return 0;
	}

	if (screenId != visual->screen) {
		std::cout << "screenId(" << screenId << ") does not match visual->screen(" << visual->screen << ").\n";
		XCloseDisplay(display);
		return 0;
	}

	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;

	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, game.settings.resolution.x, game.settings.resolution.y, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Name the window
	XStoreName(display, *window, "Named Window");

	// Create GLX OpenGL context
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");

	const char *glxExts = glXQueryExtensionsString(display, screenId);
#if 0
	std::cout << "Late extensions:\n\t" << glxExts << "\n\n";
	if (glXCreateContextAttribsARB == 0) {
		std::cout << "glXCreateContextAttribsARB() not found.\n";
	}
#endif

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,	(int)game.settings.glMajor,
		GLX_CONTEXT_MINOR_VERSION_ARB,	(int)game.settings.glMinor,
		GLX_CONTEXT_FLAGS_ARB,		0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	GLXContext context = 0;
	if (!isExtensionSupported(glxExts, "GLX_ARB_create_context")) {
		context = glXCreateNewContext(display, bestFbc, GLX_RGBA_TYPE, 0, True);
	}
	else {
		context = glXCreateContextAttribsARB(display, bestFbc, 0, true, context_attribs);
	}

	if (context == NULL) {
		std::cout << "Null Context!" << "\n";
		return 0;
	}


	XSync(display, False);

	// Verifying that context is a direct context
	if (!glXIsDirect(display, context)) {
		std::cout << "Indirect GLX rendering context obtained\n";
	}

	//if (!glXMakeCurrent(display, window, context))
	if (!glXMakeContextCurrent(display, *window, *window, context))
		std::cout << "GLX Make Current Failed!!" << "\n";

	// Show the window
	XClearWindow(display, *window);
	XMapRaised(display, *window);


	// Resize window
	/*unsigned int change_values = CWWidth | CWHeight;
	XWindowChanges values;
	values.width = game.settings.resolution.x;
	values.height = game.settings.resolution.y;
	XConfigureWindow(display, window, change_values, &values);*/
	return true;
}

void GraphicsWrapper::SetWindowContext(Display* dpy, Window *wnd, Screen* scrn, int id) {
	display = dpy;
	window = wnd;
	screen = scrn;
	sreenID = id;
}


void GraphicsWrapper::SwapBuffer() {
	glXSwapBuffers(display, *window);
}

//#endif