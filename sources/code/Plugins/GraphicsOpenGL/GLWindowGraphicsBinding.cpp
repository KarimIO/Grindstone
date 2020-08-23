#include <GL/gl3w.h>
#include "GLWindowGraphicsBinding.hpp"

#ifdef _WIN32
#include <Common/Window/Win32Window.hpp>
#include "wglext.hpp"
#endif

namespace Grindstone {
	namespace GraphicsAPI {
		bool GLWindowGraphicsBinding::initialize(Window *window) {
#ifdef _WIN32
			window_ = window;
			window_handle_ = ((Win32Window*)window)->getHandle();
			static	PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,								
				PFD_DRAW_TO_WINDOW |
				PFD_SUPPORT_OPENGL |
				PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,	
				32,	
				0, 0, 0, 0, 0, 0,
				0,				
				0,				
				0,				
				0, 0, 0, 0,		
				24,				
				8,				
				0,				
				PFD_MAIN_PLANE,	
				0,								
				0, 0, 0							
			};

			if (!(window_device_context_ = GetDC(window_handle_)))
				return false;

			unsigned int PixelFormat;
			if (!(PixelFormat = ChoosePixelFormat(window_device_context_, &pfd)))
				return false;

			if (!SetPixelFormat(window_device_context_, PixelFormat, &pfd))
				return false;

			HGLRC temp = wglCreateContext(window_device_context_);
			if (!temp) {
				return false;
			}

			if (wglMakeCurrent(window_device_context_, temp) == NULL) {
				return false;
			}


			int major = 4;
			int minor = 6;
			int attribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, major,
				WGL_CONTEXT_MINOR_VERSION_ARB, minor,
				WGL_CONTEXT_FLAGS_ARB,
				WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			}; //  | WGL_CONTEXT_DEBUG_BIT_ARB

			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
			wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
				wglGetProcAddress("wglCreateContextAttribsARB");
			if (wglCreateContextAttribsARB != NULL)
				window_render_context_ = wglCreateContextAttribsARB(window_device_context_, 0, attribs);

			PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT != NULL)
				wglSwapIntervalEXT(true);


			if (!window_render_context_) {
				window_render_context_ = temp;
			}
			else
			{
				wglMakeCurrent(window_device_context_, window_render_context_);
				wglDeleteContext(temp);
			}
#endif
			return true;
		}

		void GLWindowGraphicsBinding::shareLists(GLWindowGraphicsBinding *binding_to_copy_from) {
#ifdef _WIN32
			wglShareLists(binding_to_copy_from->window_render_context_, window_render_context_);
#endif
		}

		void GLWindowGraphicsBinding::immediateSetContext() {
#ifdef _WIN32
			wglMakeCurrent(window_device_context_, window_render_context_);
#endif
		}

		void GLWindowGraphicsBinding::immediateSwapBuffers() {
#ifdef _WIN32
			SwapBuffers(window_device_context_);
#endif
		}

		GLWindowGraphicsBinding::~GLWindowGraphicsBinding() {
#ifdef _WIN32
			wglDeleteContext(window_render_context_);
			ReleaseDC(window_handle_, window_device_context_);
#endif
		}
	};
};