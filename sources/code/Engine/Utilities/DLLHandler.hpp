#ifndef _DLL_HANDLER_H
#define _DLL_HANDLER_H

#include <string>

#if defined(_WIN32)
	#include <Windows.h>

	typedef HMODULE DLLHandle; 
#elif defined(__linux__)
	#include <dlfcn.h>
	
	typedef void * DLLHandle;
#endif

class DLLHandler {
protected:
	void initialize(std::string load);
	void *getFunction(std::string name);
	~DLLHandler();
private:
	DLLHandle handle_;
};

#endif