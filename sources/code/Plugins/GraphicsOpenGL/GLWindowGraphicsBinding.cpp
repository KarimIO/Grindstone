#include <GL/gl3w.h>
#include "GLWindowGraphicsBinding.hpp"

#ifdef _WIN32
#include <Common/Window/Win32Window.hpp>
#include "wglext.hpp"
#endif

namespace Grindstone {
	namespace GraphicsAPI {
		bool GLWindowGraphicsBinding::Initialize(Window *window) {
#ifdef _WIN32
			window = window;
			windowHandle = ((Win32Window*)window)->GetHandle();
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

			if (!(windowDeviceContext = GetDC(windowHandle)))
				return false;

			unsigned int PixelFormat;
			if (!(PixelFormat = ChoosePixelFormat(windowDeviceContext, &pfd)))
				return false;

			if (!SetPixelFormat(windowDeviceContext, PixelFormat, &pfd))
				return false;

			HGLRC temp = wglCreateContext(windowDeviceContext);
			if (!temp) {
				return false;
			}

			if (wglMakeCurrent(windowDeviceContext, temp) == NULL) {
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
				windowRenderContext = wglCreateContextAttribsARB(windowDeviceContext, 0, attribs);

			PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT != NULL)
				wglSwapIntervalEXT(true);


			if (!windowRenderContext) {
				windowRenderContext = temp;
			}
			else
			{
				wglMakeCurrent(windowDeviceContext, windowRenderContext);
				wglDeleteContext(temp);
			}
#endif
			return true;
		}

		void GLWindowGraphicsBinding::ShareLists(GLWindowGraphicsBinding *binding_to_copy_from) {
#ifdef _WIN32
			wglShareLists(binding_to_copy_from->windowRenderContext, windowRenderContext);
#endif
		}

		bool GLWindowGraphicsBinding::AcquireNextImage() { return true; }

		void GLWindowGraphicsBinding::SubmitCommandBuffer(CommandBuffer* buffers) {}

		bool GLWindowGraphicsBinding::PresentSwapchain() { return true; }

		RenderPass* GLWindowGraphicsBinding::GetRenderPass() {
			return nullptr;
		}

		Framebuffer* GLWindowGraphicsBinding::GetCurrentFramebuffer() {
			return nullptr;
		}

		uint32_t GLWindowGraphicsBinding::GetCurrentImageIndex() {
			return 0;
		}

		uint32_t GLWindowGraphicsBinding::GetMaxFramesInFlight() {
			return 0;
		}

		void GLWindowGraphicsBinding::Resize(uint32_t width, uint32_t height) {}

		void GLWindowGraphicsBinding::ImmediateSetContext() {
#ifdef _WIN32
			wglMakeCurrent(windowDeviceContext, windowRenderContext);
#endif
		}

		void GLWindowGraphicsBinding::ImmediateSwapBuffers() {
#ifdef _WIN32
			SwapBuffers(windowDeviceContext);
#endif
		}

		GLWindowGraphicsBinding::~GLWindowGraphicsBinding() {
#ifdef _WIN32
			wglDeleteContext(windowRenderContext);
			ReleaseDC(windowHandle, windowDeviceContext);
#endif
		}
	};
};
