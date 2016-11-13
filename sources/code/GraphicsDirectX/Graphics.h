#include <iostream>

#ifdef _WINDLL
	#define EXPORT __declspec(dllexport) 
#else
	#define EXPORT __declspec(dllimport) 
#endif

extern "C" {
	EXPORT void CreateGraphics();
}