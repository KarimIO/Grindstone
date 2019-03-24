#include <iostream>
#include <stdio.h>
#include <cstring>
#include "../Utilities/Logger.hpp"
#include "Engine.hpp"
#include <spdlog/spdlog.h>

extern "C" {
	_declspec(dllexport) long NvOptimusEnablement = 0x00000001;
}

int main(int argc, char *argv[]) {
	// Prepare output log location
	Logger::init("../output.log");

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