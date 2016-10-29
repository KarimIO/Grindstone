#include <iostream>
#include <stdio.h>
#ifndef _MSC_VER
#include <dlfcn.h>
#endif
#include <cstring>
#include <Graphics.h>
#include <Window.h>
#include "Engine.h"

int main(int argc, char *argv[]) {
	std::cout << "Grind Engine Start" << std::endl;
#ifndef _MSC_VER
	void *lib_handle = dlopen("./bin/graphics.so", RTLD_LAZY);
	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return 1;
	}
   
	void (*fptr)(); 
	void *tempFn = dlsym(lib_handle, "CreateGraphics");
	std::memcpy(&fptr, &tempFn, sizeof fptr);
	
	char *error;
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		return 1;
	}
	fptr();
	
	dlclose(lib_handle);
	
	lib_handle = dlopen("./bin/window.so", RTLD_LAZY);
	
	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return 1;
	}
	GameWindow* (*createWindow)();
	void (*destroyObject)(GameWindow*);

	createWindow = (GameWindow* (*)())dlsym(lib_handle, "createWindow");
	destroyObject = (void (*)(GameWindow*))dlsym(lib_handle, "destroyObject");
#else
	CreateGraphics();
#endif

	GameWindow* myClass = (GameWindow*)createWindow();
	myClass->Initialize();
	destroyObject( myClass );

#ifdef _MSC_VER
	system("pause");
#endif
	return 0;
}