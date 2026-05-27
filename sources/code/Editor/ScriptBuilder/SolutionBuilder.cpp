#include <fstream>

#include "Editor/EditorManager.hpp"
#include "SolutionBuilder.hpp"
using namespace Grindstone::Editor::ScriptBuilder;

void Grindstone::Editor::ScriptBuilder::CreateSolutionFile(const std::filesystem::path& slnPath, const std::vector<std::filesystem::path>& projects) {
	std::ofstream outputFileStream(slnPath);

	if (!outputFileStream.is_open()) {
		return;
	}

	outputFileStream.clear();
	outputFileStream << "<Solution>\n";
	for (const std::filesystem::path& projectPath : projects) {
		outputFileStream << "\t<Project Path=\"" << projectPath.string() << "\" />\n";
	}
	outputFileStream << "</Solution>\n";
	outputFileStream.close();
}
