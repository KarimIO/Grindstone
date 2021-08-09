#include <iostream>
#include "EditorManager.hpp"
#include "Converters/ShaderImporter.hpp"
using namespace Grindstone;

int main() {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	editorManager.Initialize();
	editorManager.Run();

	return 1;
}
