#include <iostream>
#include <stdio.h>
#include <Editor/CoreEditor/EditorManager.hpp>

#ifdef _WIN32
extern "C" {
	_declspec(dllexport) long NvOptimusEnablement = 0x00000001;
}
#endif

int main(int argc, char* argv[]) {
	// Prepare output log location
	Logger::init("../log/editor_output.log");

	try {
		Grindstone::EditorManager editor;
		editor.initialize();
		editor.run();
		editor.cleanup();
	}
	catch (std::runtime_error & e) {
		GRIND_FATAL(e.what());
		return EXIT_FAILURE;
	}

	return 0;
}