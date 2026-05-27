#include <fstream>

#include "CSharpProjectBuilder.hpp"
#include "Editor/EditorManager.hpp"

void Grindstone::Editor::ScriptBuilder::CreateProjectFile(const std::filesystem::path& projectPath, const std::vector<std::filesystem::path>& references) {
	std::ofstream outputFileStream(projectPath);

	if (!outputFileStream.is_open()) {
		return;
	}

	outputFileStream.clear();
	outputFileStream << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
	outputFileStream << "\t<PropertyGroup>\n";
	outputFileStream << "\t\t<TargetFramework>net10.0</TargetFramework>\n";
	outputFileStream << "\t\t<ImplicitUsings>enable</ImplicitUsings>\n";
	outputFileStream << "\t\t<Nullable>enable</Nullable>\n";
	outputFileStream << "\t</PropertyGroup>\n";

	if (!references.empty()) {
		outputFileStream << "\t<ItemGroup>\n";
		for (const std::filesystem::path& reference : references) {
			outputFileStream << "\t\t<ProjectReference Include=\"" << reference.string() << "\" />\n";
		}
		outputFileStream << "\t</ItemGroup>\n";
	}

	outputFileStream << "</Project>\n";

	outputFileStream.close();
}
