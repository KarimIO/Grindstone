#include <iostream>
#include <imgui.h>
#include "Common/Window/WindowManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/EditorManager.hpp"
#include "BuildPopup.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void BuildPopup::StartBuild() {
	auto windowManager = Editor::Manager::GetEngineCore().windowManager;
	auto window = windowManager->GetWindowByIndex(0);
	auto defaultBuildPath = Editor::Manager::GetInstance().GetProjectPath().string();
	targetPath = window->BrowseFolder(defaultBuildPath);

	if (targetPath.empty()) {
		return;
	}

	CopyBinaries();
	CopyMetaData();
	CopyCompiledAssets();
}

void BuildPopup::Render() {
	if (isShowing) {
		ImGui::Begin("Building...");
		ImGui::ProgressBar(0.5);
		ImGui::End();
	}
}

void BuildPopup::CopyBinaries() {
	sourceBuildPath = "";
	targetBuildPath = targetPath / "bin";
	std::filesystem::create_directories(targetBuildPath);
	CopyExecutableFile("Application");
	CopyDLLFile("EngineCore");
	CopyDLLFile("fmtd");
	CopyDLLFile("gl3w");
	CopyDLLFile("mono-2.0-sgen");
	CopyDLLFile("OpenAL32");
	CopyDLLFile("spdlogd");

	// TODO: Build from assets folders
	CopyDLLFile("Application-CSharp");
	CopyDLLFile("GrindstoneCSharpCore");

	// TODO: Build with all necessary graphics dlls
	CopyDLLFile("PluginGraphicsOpenGL");

	CopyPlugins();
}

void BuildPopup::CopyPlugins() {
	auto& editorManager = Editor::Manager::GetInstance();
	std::filesystem::path prefabListFile = editorManager.GetProjectPath() / "buildSettings/pluginsManifest.txt";
	auto prefabListFilePath = prefabListFile.string();
	auto fileContents = Utils::LoadFileText(prefabListFilePath.c_str());

	size_t start = 0, end;
	std::string pluginName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			pluginName = fileContents.substr(start);
			if (!pluginName.empty()) {
				CopyDLLFile(pluginName);
			}

			break;
		}

		pluginName = fileContents.substr(start, end - start);
		CopyDLLFile(pluginName.c_str());
		start = end + 1;
	}
}

void BuildPopup::CopyMetaData() {
	sourceBuildPath = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings";
	targetBuildPath = targetPath / "buildSettings";
	std::filesystem::create_directories(targetBuildPath);

	CopyBuildFile("pluginsManifest.txt");
	CopyBuildFile("scenesManifest.txt");
}

void BuildPopup::CopyBuildFile(std::string filename) {
	std::filesystem::path src = sourceBuildPath / filename;
	if (!std::filesystem::exists(src)) {
		return;
	}

	std::filesystem::path dst = targetBuildPath / filename;
	std::filesystem::copy(src, dst, std::filesystem::copy_options::update_existing);
}

void BuildPopup::CopyExecutableFile(std::string filename) {
	CopyBuildFile(filename + ".exe");
}

void BuildPopup::CopyDLLFile(std::string filename) {
	CopyBuildFile(filename + ".dll");
}

void BuildPopup::CopyCompiledAssets() {
	std::filesystem::path sourcePath = Editor::Manager::GetInstance().GetProjectPath() / "compiledAssets";
	std::filesystem::path targetAssetPath = targetPath / "compiledAssets";
	std::filesystem::remove_all(targetAssetPath);
	std::filesystem::create_directories(targetAssetPath);

	std::filesystem::copy(
		sourcePath,
		targetAssetPath,
		std::filesystem::copy_options::recursive |
		std::filesystem::copy_options::update_existing
	);
}
