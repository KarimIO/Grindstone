#include <iostream>

#ifdef _MSC_VER
	#ifdef _WINDLL
		#define EXPORT __declspec(dllexport) 
	#else
		#define EXPORT __declspec(dllimport) 
	#endif
#else
	#define EXPORT extern
#endif

extern "C" {
	EXPORT void createWindow();
}