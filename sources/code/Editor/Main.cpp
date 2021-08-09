#include <iostream>
#include "EditorManager.hpp"
#include "Converters/ShaderImporter.hpp"
using namespace Grindstone;

int main() {
	Grindstone::Editor::Manager editorManager;
	editorManager.Initialize();
	editorManager.Run();

	return 1;
}
