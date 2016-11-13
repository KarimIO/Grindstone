#ifdef _WIN32
//#include "wglext.h"
#include "Graphics.h"

bool GraphicsWrapper::InitializeWindowContext() {
	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
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

	if (!(hRC = wglCreateContext(hDC)))
		return false;

	if (!wglMakeCurrent(hDC, hRC))
		return false;

	return TRUE;
}

void GraphicsWrapper::SetWindowContext(HWND hwnd) {
	window_handle = hwnd;
}

#endif