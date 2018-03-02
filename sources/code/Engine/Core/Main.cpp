#include <iostream>
#include <stdio.h>
#include <cstring>
#include "Engine.hpp"

int main(int argc, char *argv[]) {
	std::cout << "The Grindstone Engine is Initializing.\n";
	try {
		if (!engine.Initialize()) {
#ifdef _WIN32
			system("pause");
#endif
			return -1;
		}
		
		engine.Run();
		engine.Shutdown();
	}
	catch (std::runtime_error& e) {
		Print(PRINT_FATAL_ERROR, e.what());
		return EXIT_FAILURE;
	}

	return 0;
}