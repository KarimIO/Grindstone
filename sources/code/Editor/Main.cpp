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

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ INT nCmdShow) {
	std::string projectPath = lpCmdLine;
#else
int main(int argc, char* argv[]) {
	if (argc < 2) {
		return 0;
	}

	std::string projectPath = argv[1];
#endif

	if (projectPath.empty()) {
		return 0;
	}

	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	if (editorManager.Initialize(projectPath.c_str())) {
		editorManager.Run();
	}

	return 1;
}
