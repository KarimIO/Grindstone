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

	if (window == nullptr) {
		Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Error, "BuildPopup::StartBuild - Could not get window.");
		return;
	}

	auto defaultBuildPath = Editor::Manager::GetInstance().GetProjectPath();
	targetPath = window->BrowseFolder(defaultBuildPath);

	if (targetPath.empty()) {
		return;
	}

	auto& engine = Grindstone::Editor::Manager::GetEngineCore();
	engineBinPath = engine.GetEngineBinaryPath();
	projectBinPath = engine.GetBinaryPath();
	targetBinPath = targetPath / "bin";
	sourceCompiledAssetsPath = engine.GetAssetsPath();
	targetCompiledAssetsPath = targetPath / "compiledAssets";
	sourceBuildSettingsPath = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings";
	targetBuildSettingsPath = targetPath / "buildSettings";

	std::filesystem::create_directories(targetBinPath);
	std::filesystem::create_directories(targetBuildSettingsPath);
	std::filesystem::create_directories(targetCompiledAssetsPath);

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
	CopyExecutableFile("Application");
	CopyEngineDLLFile("EngineCore");
	CopyEngineDLLFile("fmtd");
	CopyEngineDLLFile("gl3w");
	CopyEngineDLLFile("mono-2.0-sgen");
	CopyEngineDLLFile("OpenAL32");
	CopyEngineDLLFile("spdlogd");

	// TODO: Build from assets folders
	CopyProjectDLLFile("Application-CSharp");
	CopyEngineDLLFile("GrindstoneCSharpCore");

	// TODO: Build with all necessary graphics dlls
	CopyEngineDLLFile("PluginGraphicsOpenGL");
	CopyEngineDLLFile("PluginGraphicsVulkan");

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
				CopyEngineDLLFile(pluginName);
			}

			break;
		}

		pluginName = fileContents.substr(start, end - start);
		CopyEngineDLLFile(pluginName.c_str());
		start = end + 1;
	}
}

void BuildPopup::CopyMetaData() {
	CopyFile(sourceBuildSettingsPath, targetBuildSettingsPath, "pluginsManifest.txt");
	CopyFile(sourceBuildSettingsPath, targetBuildSettingsPath, "scenesManifest.txt");
}

void BuildPopup::CopyFile(std::filesystem::path srcBase, std::filesystem::path dstBase, const char* file) {
	std::filesystem::path src = srcBase / file;
	std::filesystem::path dst = dstBase / file;

	if (!std::filesystem::exists(src)) {
		return;
	}

	std::filesystem::copy(src, dst, std::filesystem::copy_options::update_existing);
}

void BuildPopup::CopyExecutableFile(std::string filename) {
	std::string filenameWithExtension = filename + ".exe";
	CopyFile(engineBinPath, targetBinPath, filenameWithExtension.c_str());
}

void BuildPopup::CopyEngineDLLFile(std::string filename) {
	std::string filenameWithExtension = filename + ".dll";
	CopyFile(engineBinPath, targetBinPath, filenameWithExtension.c_str());
}

void BuildPopup::CopyProjectDLLFile(std::string filename) {
	std::string filenameWithExtension = filename + ".dll";
	CopyFile(projectBinPath, targetBinPath, filenameWithExtension.c_str());
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
