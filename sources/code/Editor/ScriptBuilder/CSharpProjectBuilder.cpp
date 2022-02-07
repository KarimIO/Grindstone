#include <fstream>

#include "CSharpProjectBuilder.hpp"
#include "Editor/EditorManager.hpp"
using namespace Grindstone::Editor::ScriptBuilder;

CSharpProjectBuilder::CSharpProjectBuilder(
	CSharpProjectMetaData metaData
) : assemblyName(metaData.assemblyName),
	guid(metaData.assemblyGuid) {
}

void CSharpProjectBuilder::AddCodeFile(std::filesystem::path& fileName) {
	codeFiles.push_back(fileName);
}

void CSharpProjectBuilder::AddNonCodeFile(std::filesystem::path& fileName) {
	nonCodeFiles.push_back(fileName);
}

void CSharpProjectBuilder::CreateProject() {
	std::string content;
	
	content = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\" ToolsVersion=\"Current\">\n";
	
	WriteProjectInfo(content);
	WriteCodeFiles(content);
	WriteTargets(content);

	content += "</Project>\n";

	OutputFile(content);
}

void CSharpProjectBuilder::OutputFile(std::string& outputContent) {
	std::string filename = assemblyName + ".csproj";
	auto outputFilePath = Editor::Manager::GetInstance().GetProjectPath() / filename;

	std::ofstream outputFileStream(outputFilePath);

	if (!outputFileStream.is_open()) {
		return;
	}

	outputFileStream.clear();
	outputFileStream << outputContent;
	outputFileStream.close();
}
	
void CSharpProjectBuilder::WriteProjectInfo(std::string& output) {
	std::string dotNetVersion = "v4.7.2";

	output += "\t<PropertyGroup>\n"
		"\t\t<AssemblyName>" + assemblyName + "</AssemblyName>\n"
		"\t\t<OutputType>Library</OutputType>\n"
		"\t\t<ProjectGuid>{" + guid + "}</ProjectGuid>\n"
		"\t\t<BaseDirectory>.\\assets</BaseDirectory>\n"
		"\t\t<OutputPath>bin\\</OutputPath>\n"
		"\t\t<TargetFrameworkVersion>" + dotNetVersion + "</TargetFrameworkVersion>\n"
		"\t</PropertyGroup>\n";
}

void CSharpProjectBuilder::WriteCodeFiles(std::string& output) {
	output +=  "\t<ItemGroup>\n";

	for (auto &codeFileName : codeFiles) {
		std::string filename = codeFileName.string();
		std::replace(filename.begin(), filename.end(), '/', '\\');
		output += "\t\t<Compile Include=\"" + filename + "\" />\n";
	}
	
	for (auto &nonCodeFile : nonCodeFiles) {
		std::string filename = nonCodeFile.string();
		std::replace(filename.begin(), filename.end(), '/', '\\');
		output += "\t\t<Compile Include=\"" + filename + "\" />\n";
	}

	output += "\t</ItemGroup>\n";
}

void CSharpProjectBuilder::WriteTargets(std::string& output) {
	output += "\t<Import Project=\"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />";
}
