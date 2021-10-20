#include <iostream>
#include "EditorManager.hpp"
using namespace Grindstone;

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main() {
#endif
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	editorManager.Initialize();
	editorManager.Run();

	return 1;
}
