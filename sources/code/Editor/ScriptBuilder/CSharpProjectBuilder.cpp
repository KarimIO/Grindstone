#include <fstream>

#include "CSharpProjectBuilder.hpp"
#include "Editor/EditorManager.hpp"
using namespace Grindstone::Editor::ScriptBuilder;

CSharpProjectBuilder::CSharpProjectBuilder(
	const CSharpProjectMetaData& metaData
) : assemblyName(metaData.assemblyName),
	guid(metaData.assemblyGuid) {
}

void CSharpProjectBuilder::AddCodeFile(const std::filesystem::path& fileName) {
	codeFiles.push_back(fileName);
}

void CSharpProjectBuilder::AddNonCodeFile(const std::filesystem::path& fileName) {
	nonCodeFiles.push_back(fileName);
}

void CSharpProjectBuilder::CreateProject() const {
	std::string content = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\" ToolsVersion=\"Current\">\n";

	WriteProjectInfo(content);
	WriteCodeFiles(content);
	WriteTargets(content);

	content += "</Project>\n";

	OutputFile(content);
}

void CSharpProjectBuilder::OutputFile(const std::string& outputContent) const {
	const std::string filename = assemblyName + ".csproj";
	const auto outputFilePath = Editor::Manager::GetInstance().GetProjectPath() / filename;

	std::ofstream outputFileStream(outputFilePath);

	if (!outputFileStream.is_open()) {
		return;
	}

	outputFileStream.clear();
	outputFileStream << outputContent;
	outputFileStream.close();
}

void CSharpProjectBuilder::WriteProjectInfo(std::string& output) const {
	const std::string dotNetVersion = "v4.8";

	output += "\t<PropertyGroup>\n"
		"\t\t<AssemblyName>" + assemblyName + "</AssemblyName>\n"
		"\t\t<OutputType>Library</OutputType>\n"
		"\t\t<ProjectGuid>{" + guid + "}</ProjectGuid>\n"
		"\t\t<BaseDirectory>.\\assets</BaseDirectory>\n"
		"\t\t<OutputPath>bin\\</OutputPath>\n"
		"\t\t<TargetFrameworkVersion>" + dotNetVersion + "</TargetFrameworkVersion>\n"
		"\t</PropertyGroup>\n";
}

void CSharpProjectBuilder::WriteCodeFiles(std::string& output) const {
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

	WriteDllReferenceByFilename(output, "GrindstoneCSharpCore.dll");

	output += "\t</ItemGroup>\n";
}

void CSharpProjectBuilder::WriteDllReferenceByFilename(std::string& output, const std::string& path) {
	const std::filesystem::path fullPath = Editor::Manager::GetInstance().GetEngineBinariesPath() / path;
	WriteDllReference(output, fullPath.string());
}

void CSharpProjectBuilder::WriteDllReference(std::string& output, const std::string& path) {
	output += "\t\t<Reference Include=\"GrindstoneCSharpCore\">\n" \
		"\t\t\t<HintPath>" + path + "</HintPath>\n" \
		"\t\t</Reference>\n";
}

void CSharpProjectBuilder::WriteTargets(std::string& output) {
	output += "\t<Import Project=\"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />\n";
}
