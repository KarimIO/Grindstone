#ifndef _WINDOW_H
#define _WINDOW_H

#include <iostream>
#if defined(WINDOW_DLL) || defined(__linux__)
#include <InputInterface.h>
#endif

#ifdef _WIN32
	#include <Windows.h>
	
	#ifdef WINDOW_DLL
		#define WINDOW_EXPORT_CLASS __declspec(dllexport) 
		#define WINDOW_EXPORT __declspec(dllexport) 
	#else
		#define WINDOW_EXPORT_CLASS __declspec(dllimport) 
		#define WINDOW_EXPORT __declspec(dllimport) 
	#endif
#else
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/keysymdef.h>
	#include <GL/glx.h>

	#define WINDOW_EXPORT_CLASS
	#define WINDOW_EXPORT extern "C"
#endif

class InputInterface;

class WINDOW_EXPORT_CLASS GameWindow {
private:
	#ifdef __linux__
		Display* display;
		Window window;
		Screen *screen;
		GLXContext	glc;
		int screenID;
	#endif
	#ifdef _WIN32
		static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		HWND	window_handle;
	#endif
	
	InputInterface *input;

	int resX;
	int resY;
public:
	~GameWindow();
	virtual bool Initialize(const char *, int, int);
	virtual void SetInputPointer(InputInterface *);
	virtual void HandleEvents();

#if defined(__linux__)
	virtual void GetHandles(Display*, Window *, Screen*, int);
	virtual void SwapBuffer();
#elif defined(_WIN32)
	virtual HWND GetHandle();
#endif

	virtual void SetCursorShown(bool);
	virtual void ResetCursor();
	virtual void SetCursor(int x, int y);
	virtual void GetCursor(int &x, int &y);
	virtual void Shutdown();
};

extern "C" {
	WINDOW_EXPORT GameWindow* createWindow();
};

#endif