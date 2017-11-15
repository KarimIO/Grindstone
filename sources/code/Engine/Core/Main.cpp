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
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}