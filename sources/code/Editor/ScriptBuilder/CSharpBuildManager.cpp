#include "Editor/EditorManager.hpp"
#include "CSharpBuildManager.hpp"
#include "CSharpProjectBuilder.hpp"
#include "SolutionBuilder.hpp"

#ifdef _MSC_VER
#include <Windows.h>
#endif

using namespace Grindstone::Editor::ScriptBuilder;

void CSharpBuildManager::FinishInitialFileProcessing() {
	CreateProjectsAndSolution();
}

void CSharpBuildManager::AddFileInitial(const std::filesystem::path& path) {
	files.emplace_back(path);
}

void CSharpBuildManager::OnFileAdded(const std::filesystem::path& path) {
	files.emplace_back(path);
	CreateProjectsAndSolution();
}

void CSharpBuildManager::OnFileMoved(
	const std::filesystem::path& originalPath,
	const std::filesystem::path& updatedPath
) {
	for (int i = 0; i < files.size(); ++i) {
		auto& file = files[i];
		if (file == originalPath) {
			files[i] = updatedPath;
			CreateProjectsAndSolution();
		}
	}

	CreateProjectsAndSolution();
}

void CSharpBuildManager::OnFileDeleted(const std::filesystem::path& path) {
	for (int i = 0; i < files.size(); ++i) {
		auto& file = files[i];
		if (file == path) {
			files.erase(files.begin() + i);
			CreateProjectsAndSolution();
		}
	}
}

void CSharpBuildManager::OnFileModified(const std::filesystem::path& path) {
	BuildProject();
}

void CSharpBuildManager::BuildProject() {
#ifdef _MSC_VER
	std::string filename = "Application-CSharp.csproj";
	auto outputFilePath = Editor::Manager::GetInstance().GetProjectPath() / filename;
	std::string path = outputFilePath.string();
	std::string msBuildPath = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Msbuild\\Current\\Bin\\MSBuild.exe";

	ShellExecute(
		NULL,
		NULL,
		msBuildPath.c_str(),
		path.c_str(),
		NULL,
		SW_HIDE
	);
#endif
}

void CSharpBuildManager::CreateProjectsAndSolution() {
	CreateProject();
	CreateSolution();

	BuildProject();
}

void CSharpBuildManager::CreateProject() {
	CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");
	
	CSharpProjectBuilder projectBuilder(metaData);

	for (auto& path : files) {
		projectBuilder.AddCodeFile(path);
	}
	
	projectBuilder.CreateProject();
}

void CSharpBuildManager::CreateSolution() {
	CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");

	SolutionBuilder solutionBuilder;
	solutionBuilder.AddProject(metaData);
	solutionBuilder.CreateSolution();
}
