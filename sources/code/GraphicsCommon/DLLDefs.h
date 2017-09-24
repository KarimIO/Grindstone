#ifndef _DEF_DLL_H
#define _DEF_DLL_H

#ifdef _WIN32
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

#endif