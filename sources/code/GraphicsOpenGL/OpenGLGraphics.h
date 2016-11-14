#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <iostream>

#ifdef _WIN32
	#include <Windows.h>

	#ifdef GRAPHICS_DLL
		#define GRAPHICS_EXPORT_CLASS __declspec(dllexport) 
		#define GRAPHICS_EXPORT __declspec(dllexport) 
	#else
		#define GRAPHICS_EXPORT_CLASS __declspec(dllimport) 
		#define GRAPHICS_EXPORT __declspec(dllimport) 
	#endif
#else
	#define GRAPHICS_EXPORT_CLASS
	#define GRAPHICS_EXPORT extern "C"
#endif

#ifdef __linux__
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/keysymdef.h>
	#include <GL/glx.h>
#endif

class InputInterface;

class GRAPHICS_EXPORT_CLASS GraphicsWrapper {
private:
#ifdef _WIN32
	HWND	window_handle;
	HGLRC	hRC;
	HDC		hDC;
#elif __linux__
	Display* display;
	Window *window;
	Screen* screen;
	int screenId;
#endif
public:
	virtual bool InitializeWindowContext();
	virtual bool InitializeGraphics();
	virtual void SwapBuffer();
#ifdef _WIN32
	virtual void SetWindowContext(HWND);
#elif __linux__
	virtual void SetWindowContext(Display*, Window *window, Screen* screen, int screenId);
#endif
};

extern "C" {
	GRAPHICS_EXPORT GraphicsWrapper* createGraphics();
};

#endif