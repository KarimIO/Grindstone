#include <iostream>
#include <stdio.h>
#include <cstring>
#include <Window.h>
#include "Core/Input.h"
#include "Engine.h"

int main(int argc, char *argv[]) {
	std::cout << "The Grindstone Engine is Initializing.\n";

	if (!engine.Initialize()) {
#ifdef _WIN32
		system("pause");
#endif
		return -1;
	}
		
	engine.Run();
	engine.Shutdown();

	return 0;
}