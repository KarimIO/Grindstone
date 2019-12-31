#if (defined(__linux__) || defined(__APPLE__))
//#include <GL/gl3w.h>
#include "GLGraphicsWrapper.hpp"
#include <GL/glx.h>

#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

		static bool isExtensionSupported(const char *extList, const char *extension) {
			const char *start;
			const char *where, *terminator;

			where = strchr(extension, ' ');
			if (where || *extension == '\0') {
				return false;
			}

			for (start = extList;;) {
				where = strstr(start, extension);

				if (!where) {
					break;
				}

				terminator = where + strlen(extension);

				if (where == start || *(where - 1) == ' ') {
					if (*terminator == ' ' || *terminator == '\0') {
						return true;
					}
				}

				start = terminator;
			}

			return false;
		}

		void GLGraphicsWrapper::CleanX11() {
			glXDestroyContext(xDisplay, context);
			//XFreeColormap(xDisplay, windowAttribs.colormap);
			XDestroyWindow(xDisplay, xWindow);
			XCloseDisplay(xDisplay);
		}

		bool GLGraphicsWrapper::InitializeWindowContext() {
			Screen* screen;
			int screenId;
			XEvent ev;

			// Open the display
			xDisplay = XOpenDisplay(NULL);
			if (xDisplay == NULL) {
				std::cout << "Could not open display\n";
				return 1;
			}
			screen = DefaultScreenOfDisplay(xDisplay);
			screenId = DefaultScreen(xDisplay);

			// Check GLX version
			GLint majorGLX, minorGLX = 0;
			glXQueryVersion(xDisplay, &majorGLX, &minorGLX);
			if (majorGLX <= 1 && minorGLX < 2) {
				std::cout << "GLX 1.2 or greater is required.\n";
				XCloseDisplay(xDisplay);
				return 1;
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
				GLX_STENCIL_SIZE    , 0,
				GLX_DOUBLEBUFFER    , True,
				None
			};

			int fbcount;
			GLXFBConfig* fbc = glXChooseFBConfig(xDisplay, screenId, glxAttribs, &fbcount);
			if (fbc == 0) {
				std::cout << "Failed to retrieve framebuffer.\n";
				XCloseDisplay(xDisplay);
				return 1;
			}

			// Pick the FB config/visual with the most samples per pixel
			int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
			for (int i = 0; i < fbcount; ++i) {
				XVisualInfo *vi = glXGetVisualFromFBConfig(xDisplay, fbc[i]);
				if (vi != 0) {
					int samp_buf, samples;
					glXGetFBConfigAttrib(xDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
					glXGetFBConfigAttrib(xDisplay, fbc[i], GLX_SAMPLES, &samples);

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
			GLXFBConfig bestFbc = fbc[best_fbc];
			XFree(fbc); // Make sure to free this!


			XVisualInfo* visual = glXGetVisualFromFBConfig(xDisplay, bestFbc);
			if (visual == 0) {
				std::cout << "Could not create correct visual window.\n";
				XCloseDisplay(xDisplay);
				return 1;
			}

			if (screenId != visual->screen) {
				std::cout << "screenId(" << screenId << ") does not match visual->screen(" << visual->screen << ").\n";
				XCloseDisplay(xDisplay);
				return 1;

			}

			// Open the window
			XSetWindowAttributes windowAttribs;
			windowAttribs.border_pixel = BlackPixel(xDisplay, screenId);
			windowAttribs.background_pixel = WhitePixel(xDisplay, screenId);
			windowAttribs.override_redirect = True;
			windowAttribs.colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, screenId), visual->visual, AllocNone);
			windowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask;
			xWindow = XCreateWindow(xDisplay, RootWindow(xDisplay, screenId), 0, 0, width, height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

			// Redirect Close
			Atom atomWmDeleteWindow = XInternAtom(xDisplay, "WM_DELETE_WINDOW", False);
			XSetWMProtocols(xDisplay, xWindow, &atomWmDeleteWindow, 1);

			// Create GLX OpenGL context
			glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
			glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");

			int context_attribs[] = {
				GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
				GLX_CONTEXT_MINOR_VERSION_ARB, 6,
				GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB,
				None
			};

			context = 0;
			const char *glxExts = glXQueryExtensionsString(xDisplay, screenId);
			if (!isExtensionSupported(glxExts, "GLX_ARB_create_context")) {
				std::cout << "GLX_ARB_create_context not supported\n";
				context = glXCreateNewContext(xDisplay, bestFbc, GLX_RGBA_TYPE, 0, True);
			}
			else {
				context = glXCreateContextAttribsARB(xDisplay, bestFbc, 0, true, context_attribs);
			}
			XSync(xDisplay, False);
			XStoreName(xDisplay, xWindow, title);

			// Verifying that context is a direct context
			if (!glXIsDirect(xDisplay, context)) {
				std::cout << "Indirect GLX rendering context obtained\n";
			}
			else {
				std::cout << "Direct GLX rendering context obtained\n";
			}
			glXMakeCurrent(xDisplay, xWindow, context);

			// Show the window
			XClearWindow(xDisplay, xWindow);
			XMapRaised(xDisplay, xWindow);

			//XFree(visual);

			return true;
		}
	}
}

#endif