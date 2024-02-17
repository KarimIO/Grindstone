#include <fstream>

#include "Editor/EditorManager.hpp"
#include "SolutionBuilder.hpp"
using namespace Grindstone::Editor::ScriptBuilder;

void SolutionBuilder::AddProject(const CSharpProjectMetaData& projectMetaData) {
	projects.emplace_back(projectMetaData.assemblyName, projectMetaData.assemblyGuid);
}

void SolutionBuilder::CreateSolution() {
	std::string content =
		"Microsoft Visual Studio Solution File, Format Version 11.00\n"
		"# Visual Studio 2010\n";

	WriteMainProjectSection(content);

	content += "Global\n";
	WriteSolutionConfigs(content);
	WriteProjectConfigs(content);
	WriteSolutionProperties(content);
	content += "EndGlobal\n";

	OutputFile(content);
}

void SolutionBuilder::OutputFile(const std::string& output) {
	const std::string filename = "Application.sln";
	const std::filesystem::path outputFilePath = Editor::Manager::GetInstance().GetProjectPath() / filename;

	std::ofstream outputFileStream(outputFilePath);

	if (!outputFileStream.is_open()) {
		return;
	}

	outputFileStream.clear();
	outputFileStream << output;
	outputFileStream.close();
}

void SolutionBuilder::WriteMainProjectSection(std::string& output) const {
	for (auto& project : projects) {
		output +=
			"Project(\"{" + project.assemblyGuid + "}\") = \"" + project.assemblyName + "\", \"" + project.assemblyName + ".csproj\", \"{" + project.assemblyGuid + "}\"\n"
			"EndProject\n";
	}
}

void SolutionBuilder::WriteSolutionConfigs(std::string& output) {
	output +=
		"\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
		"\t\tDebug|Any CPU = Debug|Any CPU\n"
		"\t\tRelease|Any CPU = Release|Any CPU\n"
		"\tEndGlobalSection\n";
}

void SolutionBuilder::WriteProjectConfigs(std::string& output) {
	output += "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (auto& project : projects) {
		output +=
			"\t\t{" + project.assemblyGuid + "}.Debug|Any CPU.ActiveCfg = Debug|Any CPU\n"
			"\t\t{" + project.assemblyGuid + "}.Debug|Any CPU.Build.0 = Debug|Any CPU\n"
			"\t\t{" + project.assemblyGuid + "}.Release|Any CPU.ActiveCfg = Release|Any CPU\n"
			"\t\t{" + project.assemblyGuid + "}.Release|Any CPU.Build.0 = Release|Any CPU\n";
	}

	output += "\tEndGlobalSection\n";
}

void SolutionBuilder::WriteSolutionProperties(std::string& output) {
	output +=
		"\tGlobalSection(SolutionProperties) = preSolution\n"
		"\t\tHideSolutionNode = FALSE\n"
		"\tEndGlobalSection\n";
}
