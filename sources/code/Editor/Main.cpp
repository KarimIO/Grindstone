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
#else
int main() {
#endif
	std::string projectPath = "D:/Work/InOrdinate/Grindstone/Sandbox/";

	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	if (editorManager.Initialize(projectPath.c_str())) {
		editorManager.Run();
	}

	return 1;
}
