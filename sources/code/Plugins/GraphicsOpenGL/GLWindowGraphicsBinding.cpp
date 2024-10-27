#include <GL/gl3w.h>
#include "GLWindowGraphicsBinding.hpp"

/*
// TODO: Reimplement Win32
#ifdef _WIN32
#include <Common/Window/Win32Window.hpp>
#include <GL/wglext.h>
#endif
*/

#include <Common/Window/GlfwWindow.hpp>

using namespace Grindstone::GraphicsAPI;

bool OpenGL::WindowGraphicsBinding::Initialize(Window *newWindow) {
	/*
#ifdef _WIN32
	window = newWindow;
	windowHandle = static_cast<Win32Window*>(window)->GetHandle();
	static PIXELFORMATDESCRIPTOR pfd =
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
	*/

	return true;
}

void OpenGL::WindowGraphicsBinding::ShareLists(OpenGL::WindowGraphicsBinding *bindingToCopyFrom) {
#ifdef _WIN32
	wglShareLists(bindingToCopyFrom->windowRenderContext, windowRenderContext);
#endif
}

bool OpenGL::WindowGraphicsBinding::AcquireNextImage() { return true; }

void OpenGL::WindowGraphicsBinding::SubmitCommandBuffer(CommandBuffer* buffers) {}

bool OpenGL::WindowGraphicsBinding::PresentSwapchain() { return true; }

Grindstone::GraphicsAPI::RenderPass* OpenGL::WindowGraphicsBinding::GetRenderPass() {
	return nullptr;
}

Grindstone::GraphicsAPI::Framebuffer* OpenGL::WindowGraphicsBinding::GetCurrentFramebuffer() {
	return nullptr;
}

uint32_t OpenGL::WindowGraphicsBinding::GetCurrentImageIndex() {
	return 0;
}

uint32_t OpenGL::WindowGraphicsBinding::GetMaxFramesInFlight() {
	return 0;
}

void OpenGL::WindowGraphicsBinding::Resize(uint32_t width, uint32_t height) {}

void OpenGL::WindowGraphicsBinding::ImmediateSetContext() {
#ifdef _WIN32
	wglMakeCurrent(windowDeviceContext, windowRenderContext);
#endif
}

void OpenGL::WindowGraphicsBinding::ImmediateSwapBuffers() {
#ifdef _WIN32
	SwapBuffers(windowDeviceContext);
#endif
}

OpenGL::WindowGraphicsBinding::~WindowGraphicsBinding() {
#ifdef _WIN32
	wglDeleteContext(windowRenderContext);
	ReleaseDC(windowHandle, windowDeviceContext);
#endif
}
