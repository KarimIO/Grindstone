#include <iostream>

#ifdef _MSC_VER
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

	#define WINDOW_EXPORT_CLASS
	#define WINDOW_EXPORT extern "C"
#endif

class WINDOW_EXPORT_CLASS GameWindow {
private:
	#ifdef __linux__
		Display* display;
		Window window;
		Screen *screen;
	#endif
public:
	~GameWindow();
	virtual bool Initialize();
};

WINDOW_EXPORT GameWindow* createWindow();
WINDOW_EXPORT void destroyObject( GameWindow* object );