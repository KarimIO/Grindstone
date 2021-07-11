#include <iostream>
#include "EditorManager.hpp"
#include "Converters/ShaderImporter.hpp"
using namespace Grindstone;

int main() {
	Grindstone::Editor::Manager editorManager;
	editorManager.initialize();
	editorManager.run();

	return 1;
}
