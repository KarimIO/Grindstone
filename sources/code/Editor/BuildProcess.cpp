#include <iostream>

#include "Common/Window/WindowManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/EditorManager.hpp"
#include "AssetPackSerializer.hpp"
#include "BuildProcess.hpp"

namespace Grindstone::Editor {
	static void CopyFileTo(std::filesystem::path srcBase, std::filesystem::path dstBase, std::string_view file) {
		std::filesystem::path src = srcBase / file;
		std::filesystem::path dst = dstBase / file;

		if (!std::filesystem::exists(src)) {
			return;
		}

		std::filesystem::copy(src, dst, std::filesystem::copy_options::update_existing);
	}

	static void CopyLibrary(std::filesystem::path srcBase, std::filesystem::path dstBase, std::string_view file) {
		std::string filename = std::string(file) + ".dll";
		std::filesystem::copy(srcBase / filename, dstBase / filename, std::filesystem::copy_options::update_existing);
	}

	static void CopyExecutable(std::filesystem::path srcBase, std::filesystem::path dstBase, std::string_view file) {
		std::string filename = std::string(file) + ".exe";
		std::filesystem::copy(srcBase / filename, dstBase / filename, std::filesystem::copy_options::update_existing);
	}

	static void CopyPlugins(std::filesystem::path& enginePath, std::filesystem::path& targetPath) {
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
					CopyLibrary(enginePath, targetPath, pluginName);
				}

				break;
			}

			pluginName = fileContents.substr(start, end - start);
			CopyLibrary(enginePath, targetPath, pluginName);
			start = end + 1;
		}
	}

	static void CopyBinaries(std::filesystem::path& enginePath, std::filesystem::path& projectPath, std::filesystem::path& targetPath) {
		CopyExecutable(enginePath, targetPath, "Application");
		CopyLibrary(enginePath, targetPath, "EngineCore");
		CopyLibrary(enginePath, targetPath, "fmtd");
		CopyLibrary(enginePath, targetPath, "gl3w");
		CopyLibrary(enginePath, targetPath, "glfw3");
		CopyLibrary(enginePath, targetPath, "mono-2.0-sgen");
		CopyLibrary(enginePath, targetPath, "OpenAL32");
		CopyLibrary(enginePath, targetPath, "spdlogd");

		// TODO: Build from assets folders
		CopyLibrary(projectPath, targetPath, "Application-CSharp");
		CopyLibrary(projectPath, targetPath, "GrindstoneCSharpCore");

		// TODO: Build with all necessary graphics dlls
		CopyLibrary(enginePath, targetPath, "PluginGraphicsVulkan");

		CopyPlugins(enginePath, targetPath);
	}

	static void CopyMetaData(std::filesystem::path& sourceBuildSettingsPath, std::filesystem::path& targetBuildSettingsPath) {
		CopyFileTo(sourceBuildSettingsPath, targetBuildSettingsPath, "pluginsManifest.txt");
		CopyFileTo(sourceBuildSettingsPath, targetBuildSettingsPath, "scenesManifest.txt");
	}

	static std::filesystem::path GetBuildPath() {
		WindowManager* windowManager = Editor::Manager::GetEngineCore().windowManager;
		Window* window = windowManager->GetWindowByIndex(0);

		if (window == nullptr) {
			Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Error, "StartBuild - Could not get window.");
			return "";
		}

		std::filesystem::path defaultBuildPath = Editor::Manager::GetInstance().GetProjectPath();
		return window->BrowseFolder(defaultBuildPath);
	}

	void BuildGame(BuildProcessStats* processStatus) {
		if (processStatus == nullptr) {
			return;
		}

		const std::filesystem::path targetPath = GetBuildPath();

		if (targetPath.empty()) {
			return;
		}

		const EngineCore& engine = Grindstone::Editor::Manager::GetEngineCore();
		std::filesystem::path engineBinPath = engine.GetEngineBinaryPath();
		std::filesystem::path projectBinPath = engine.GetBinaryPath();
		std::filesystem::path sourceBuildSettingsPath = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings";

		std::filesystem::path targetBinPath = targetPath / "bin";
		std::filesystem::path targetCompiledAssetsPath = targetPath / "archives";
		std::filesystem::path targetBuildSettingsPath = targetPath / "buildSettings";

		std::filesystem::create_directories(targetBinPath);
		std::filesystem::create_directories(targetBuildSettingsPath);
		std::filesystem::create_directories(targetCompiledAssetsPath);

		{
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Copying Binaries";
			processStatus->detailText = "";
			processStatus->progress = 0.0f;
		}
		CopyBinaries(engineBinPath, projectBinPath, targetBinPath);
		{
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Copying Meta Assets";
			processStatus->detailText = "";
		}
		CopyMetaData(sourceBuildSettingsPath, targetBuildSettingsPath);
		{
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Compiling Asset Archives";
			processStatus->progress = 0.25f;
		}
		Grindstone::Assets::AssetPackSerializer::SerializeAllAssets(targetCompiledAssetsPath, processStatus, 0.25f, 0.75f);
		{
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Done";
			processStatus->progress = 1.0f;
		}
	}
}
