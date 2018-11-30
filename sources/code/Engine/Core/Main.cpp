#include <iostream>
#include <stdio.h>
#include <cstring>
#include "../Utilities/Logger.hpp"
#include "Engine.hpp"

/*extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}*/


int main(int argc, char *argv[]) {
	// Prepare output log location
	Logging::Logger::getInstance().setPath("./output.log");

	try {
		// Note that engine calls its constructor, Engine::Engine
		engine.initialize();
		engine.run();
	}
	catch (std::runtime_error& e) {
		LOG_FATAL(e.what());
		return EXIT_FAILURE;
	}

	return 0;
}