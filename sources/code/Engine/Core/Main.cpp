#include <iostream>
#include <stdio.h>
#include "Engine.hpp"

#ifdef _WIN32
extern "C" {
	_declspec(dllexport) long NvOptimusEnablement = 0x00000001;
}
#endif

int main(int argc, char *argv[]) {
	// Prepare output log location
	Logger::init("../log/output.log");

	try {
		// Note that engine calls its constructor, Engine::Engine
		engine.initialize();
		engine.run();
	}
	catch (std::runtime_error& e) {
		GRIND_FATAL(e.what());
		return EXIT_FAILURE;
	}

	return 0;
}