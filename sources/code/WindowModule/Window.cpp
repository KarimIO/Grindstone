#include "Window.h"
#ifdef _MSC_VER
	#include "win32Window.h"
#else
	#include "x11Window.h"
#endif

EXPORT void createWindow()
{
	std::cout << "Window Module Loaded...\n";
	launchWindow();
}