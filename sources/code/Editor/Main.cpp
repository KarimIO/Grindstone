#include <iostream>
#include "EditorManager.hpp"
#include "Converters/ShaderImporter.hpp"
using namespace Grindstone;

int main() {
	Converters::ShaderImporter importer;
	importer.convertFile("../test.vert.glsl");

	Grindstone::Editor::Manager editorManager;
	editorManager.initialize();
	editorManager.run();

	return 1;
}