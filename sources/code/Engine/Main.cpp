#include <iostream>
#include <stdio.h>
#ifndef _MSC_VER
#include <dlfcn.h>
#endif
#include <cstring>
#include <Graphics.h>
#include <Window.h>
#include <Input.h>
#include "Engine.h"

int main(int argc, char *argv[]) {
	std::cout << "Grind Engine Start" << std::endl;
#ifdef  __unix__
	void *lib_handle = dlopen("./bin/graphics.so", RTLD_LAZY);
	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return 1;
	}
   
	void (*CreateGraphics)();
	CreateGraphics = (void (*)())dlsym(lib_handle, "CreateGraphics");
#endif
	CreateGraphics();

	if (!engine.Initialize())
		return -1;
		
	engine.Run();
	engine.Shutdown();
		
#ifdef __unix__
	dlclose(lib_handle);
#endif

	return 0;
}