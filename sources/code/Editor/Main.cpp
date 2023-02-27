#include <iostream>
#include "EditorManager.hpp"
using namespace Grindstone;

#ifdef _WIN32
#include <Windows.h>
// Request High-Performance GPU for Nvidia and AMD
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argc, char* argv[]) {
	std::string projectPath;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-projectpath") == 0 && argc > i + 1) {
			projectPath = argv[i + 1];
		}
	}

	if (projectPath.empty()) {
		std::cerr << "Unable to launch Grindstone Editor - no path set." << std::endl;
		return 1;
	}

	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	if (editorManager.Initialize(projectPath.c_str())) {
		editorManager.Run();
	}

	return 0;
}
