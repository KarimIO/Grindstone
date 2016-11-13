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

class InputInterface;

class GRAPHICS_EXPORT_CLASS GraphicsWrapper {
private:
	HWND	window_handle;
	HGLRC	hRC;
	HDC		hDC;
public:
#ifdef _WIN32
	virtual bool InitializeWindowContext();
	virtual void SetWindowContext(HWND);
#endif
};

extern "C" {
	GRAPHICS_EXPORT GraphicsWrapper* CreateGraphics();
};

#endif