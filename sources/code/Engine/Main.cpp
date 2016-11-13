#include <iostream>
#include <stdio.h>
#ifndef _MSC_VER
#include <dlfcn.h>
#endif
#include <cstring>
#include <Window.h>
#include <Input.h>
#include "Engine.h"

int main(int argc, char *argv[]) {
	std::cout << "Grind Engine Start" << std::endl;

	if (!engine.Initialize())
		return -1;
		
	engine.Run();
	engine.Shutdown();

	return 0;
}