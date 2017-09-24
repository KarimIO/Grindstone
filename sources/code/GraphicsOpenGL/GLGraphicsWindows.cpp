#include "GLGraphicsWrapper.h"

#if !defined(GLFW_WINDOW) && defined(_WIN32)
#include <GL/gl3w.h>
#include <windows.h>
#include "wglext.h"

bool GLGraphicsWrapper::InitializeWindowContext() {
	InitializeWin32Window();

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 24Bit Z-Buffer (Depth Buffer)  
		8,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(window_handle)))
		return false;

	unsigned int PixelFormat;
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
		return false;

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))
		return false;

	HGLRC temp = wglCreateContext(hDC);
	if (!temp) {
		std::cout << "wglCreateContext Failed!\n";
		return false;
	}

	if (wglMakeCurrent(hDC, temp) == NULL) {
		std::cout << "Make Context Current Second Failed!\n";
		return false;
	}


	int major = 3;
	int minor = 3;
	//glGetIntegerv(GL_MAJOR_VERSION, &major);
	//glGetIntegerv(GL_MINOR_VERSION, &minor);

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
		hRC = wglCreateContextAttribsARB(hDC, 0, attribs);

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
		wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT != NULL)
		wglSwapIntervalEXT(vsync);


	if (!hRC)
		hRC = temp;
	else
	{
		wglMakeCurrent(hDC, hRC);
		wglDeleteContext(temp);
	}

	return TRUE;
}

#endif