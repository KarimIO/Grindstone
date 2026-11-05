#include <atomic>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>


#include <Common/Logger.hpp>
#include <Common/Window/Window.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/PluginSystem/EditorPluginManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "AssetPackSerializer.hpp"
#include "BuildProcess.hpp"

namespace Grindstone::Editor {
	static const char* configurationPath = CMAKE_INTDIR;

	static bool CopyFileTo(
		const std::filesystem::path& srcFile,
		const std::filesystem::path& dstFile
	) {

		if (!std::filesystem::exists(srcFile)) {
			GPRINT_ERROR(LogSource::Editor, "Failed to copy '{}'.", srcFile.string().c_str());
			return false;
		}

		std::filesystem::copy(srcFile, dstFile, std::filesystem::copy_options::update_existing);
		return true;
	}
	
	static bool CopyFileTo(
		const std::filesystem::path& srcBase,
		const std::filesystem::path& dstBase,
		const std::filesystem::path& file
	) {
		const std::filesystem::path& srcFile = srcBase / file;
		const std::filesystem::path& dstFile = dstBase / file;

		if (!std::filesystem::exists(srcFile)) {
			GPRINT_ERROR(LogSource::Editor, "Failed to copy '{}'.", file.string().c_str());
			return false;
		}

		std::filesystem::copy(srcFile, dstFile, std::filesystem::copy_options::update_existing);
		return true;
	}
	
	static std::filesystem::path GetBuildPath() {
		WindowManager* windowManager = Editor::Manager::GetEngineCore().windowManager;
		Window* window = windowManager->GetWindowByIndex(0);

		GS_ASSERT_ENGINE_WITH_MESSAGE(window != nullptr, "StartBuild - Could not get window.");

		std::filesystem::path defaultBuildPath = Editor::Manager::GetInstance().GetProjectPath();
		return window->BrowseFolder(defaultBuildPath);
	}

	struct InstallCopyFile {
		std::filesystem::path source;
		std::filesystem::path destination;
		InstallCopyFile(const std::filesystem::path& path) : source(path), destination(path) {};
		InstallCopyFile(const std::filesystem::path& src, const std::filesystem::path& dst) : source(src), destination(dst) {};

		friend bool operator<(const InstallCopyFile& lhs, const InstallCopyFile& rhs) {
			return lhs.source < rhs.source && lhs.destination < rhs.destination;
		}
	};

	struct InstallCopyFolder {
		std::filesystem::path sourceFolderPath;
		std::filesystem::path destinationFolderPath;
		std::set<InstallCopyFile> installFiles;
	};

	struct InstallStage {
		std::string displayName;
		float stageSize01; // The amount of the total progress bar this stage should represent.
		std::vector<InstallCopyFolder> installFolders;
	};

	struct InstallPlan {
		std::vector<InstallStage> stages;
	};

	#define EXE_EXT ".exe"
	#define DLL_EXT ".dll"

	static InstallStage MarkAppBinaries(const std::filesystem::path& targetPath) {
		InstallStage stage;
		stage.displayName = "Application Binaries";
		stage.stageSize01 = 0.1f;

		const EngineCore& engine = Grindstone::Editor::Manager::GetEngineCore();
		const std::filesystem::path engineBinPath = engine.GetEngineBinaryPath();

		InstallCopyFolder& folder = stage.installFolders.emplace_back();
		folder.sourceFolderPath = engineBinPath / configurationPath;
		folder.destinationFolderPath = targetPath / "bin" / configurationPath;

		folder.installFiles.emplace(InstallCopyFile("Application" EXE_EXT));
		folder.installFiles.emplace(InstallCopyFile("EngineCore" DLL_EXT));

		return stage;
	}

	static InstallStage MarkPluginBinaries(const std::filesystem::path& targetPath) {
		InstallStage stage {
			.displayName = "Plugin Binaries",
			.stageSize01 = 0.1f
		};

		const EngineCore& engine = Grindstone::Editor::Manager::GetEngineCore();
		const std::filesystem::path engineBinPath = engine.GetEngineBinaryPath();
		Grindstone::Plugins::EditorPluginManager& pluginManager = *static_cast<Grindstone::Plugins::EditorPluginManager*>(engine.GetPluginManager());

		for (const Plugins::MetaData& plugin : pluginManager) {
			for (const Plugins::MetaData::Binary& binary : plugin.binaries) {
				std::string lower = Grindstone::Utils::ToLower(binary.libraryRelativePath.string());
				if (lower.find("editor") != std::string::npos) {
					continue;
				}

				std::filesystem::path binaryPath = plugin.pluginResolvedPath / binary.libraryRelativePath;
				std::filesystem::path parentPath = binaryPath.parent_path();

				InstallCopyFolder& folder = stage.installFolders.emplace_back();
				folder.sourceFolderPath = parentPath;
				folder.destinationFolderPath = targetPath / "bin" / configurationPath;

				folder.installFiles.emplace(InstallCopyFile(binary.libraryRelativePath.filename().string() + DLL_EXT));
			}
		}

		return stage;
	}

	static InstallStage MarkMetaFiles(const std::filesystem::path& targetPath) {
		InstallStage stage {
			.displayName = "Configuration Files",
			.stageSize01 = 0.1f
		};

		Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
		const std::filesystem::path engineBinPath = engineCore.GetEngineBinaryPath();

		InstallCopyFolder& folder = stage.installFolders.emplace_back();
		folder.sourceFolderPath = engineBinPath;
		folder.destinationFolderPath = targetPath / "bin" / configurationPath;

		return stage;
	}

	static bool MarkNecessaryFiles(const std::filesystem::path& targetPath, InstallPlan& installPlan) {
		const EngineCore& engine = Grindstone::Editor::Manager::GetEngineCore();
		const std::filesystem::path engineBinPath = engine.GetEngineBinaryPath();
		const std::filesystem::path projectBinPath = engine.GetBinaryPath();
		const std::filesystem::path sourceBuildSettingsPath = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings";

		const std::filesystem::path targetCompiledAssetsPath = "archives";
		const std::filesystem::path targetBuildSettingsPath = "buildSettings";

		installPlan.stages.emplace_back(MarkAppBinaries(targetPath));
		installPlan.stages.emplace_back(MarkPluginBinaries(targetPath));
		installPlan.stages.emplace_back(MarkMetaFiles(targetPath));

		return true;
	}

	static bool CommitCopies(
		BuildProcessStats* processStatus,
		const InstallPlan& installPlan
	) {
		bool isSuccessful = true;

		float accumulatedStagesProgress = 0.0f;
		for (const InstallStage& stage : installPlan.stages) {
			{
				std::scoped_lock scopedLock(processStatus->stringMutex);
				processStatus->stageText = stage.displayName;
				processStatus->detailText = "";
				processStatus->progress = accumulatedStagesProgress;
			}

			for (const InstallCopyFolder& folder : stage.installFolders) {
				float perFileProgressIncrement = stage.stageSize01 / folder.installFiles.size();

				if (!std::filesystem::exists(folder.destinationFolderPath)) {
					std::filesystem::create_directories(folder.destinationFolderPath);
				}

				for (const InstallCopyFile& file : folder.installFiles) {
					{
						std::scoped_lock scopedLock(processStatus->stringMutex);
						processStatus->detailText = file.source.string();
						processStatus->progress = accumulatedStagesProgress + perFileProgressIncrement;
					}

					if (!CopyFileTo(folder.sourceFolderPath / file.source, folder.destinationFolderPath / file.destination)) {
						isSuccessful = false;
					}
				}
			}

			accumulatedStagesProgress += stage.stageSize01;
		}

		return isSuccessful;
	}

	static bool BuildPluginManifest(const std::filesystem::path& targetPath) {
		const EngineCore& engine = Grindstone::Editor::Manager::GetEngineCore();
		const std::filesystem::path engineBinPath = engine.GetEngineBinaryPath();
		Grindstone::Plugins::EditorPluginManager& pluginManager = *static_cast<Grindstone::Plugins::EditorPluginManager*>(engine.GetPluginManager());

		std::map<std::string, std::vector<std::string>> pluginBinaries;
		for (const Plugins::MetaData& plugin : pluginManager) {
			for (const Plugins::MetaData::Binary& binary : plugin.binaries) {
				pluginBinaries[binary.loadStage].emplace_back(binary.libraryRelativePath.string());
			}
		}

		std::filesystem::path parentDir = targetPath / "buildSettings";
		if (!std::filesystem::exists(parentDir)) {
			std::filesystem::create_directories(parentDir);
		}

		std::ofstream writeStream(parentDir / "pluginsManifest.txt");
		if (!writeStream.is_open()) {
			return false;
		}

		bool hasWrittenNoStages = true;
		writeStream << "{";
		for (const auto& loadStage : pluginBinaries) {
			if (hasWrittenNoStages) {
				hasWrittenNoStages = false;
			} else {
				writeStream << ",";
			}

			writeStream << "\n\t\"" << loadStage.first << "\": [\n\t\t";
			size_t loadStateBinaryCount = loadStage.second.size();
			for (size_t i = 0; i < loadStateBinaryCount; ++i) {
				writeStream << "\"" << loadStage.second[i] << "\"";
				if (i < loadStateBinaryCount - 1) {
					writeStream << ",\n\t\t";
				}
				else {
					writeStream << "\n\t";
				}
			}
			writeStream << "]";
		}
		writeStream << "\n}\n";
		writeStream.close();

		return true;
	}

	void BuildGame(BuildProcessStats* processStatus) {
		GS_ASSERT_ENGINE(processStatus != nullptr);

		const std::filesystem::path targetPath = GetBuildPath();

		if (targetPath.empty()) {
			return;
		}

		if (std::filesystem::exists(targetPath)) {
			std::filesystem::remove_all(targetPath);
		}

		std::filesystem::create_directories(targetPath);

		InstallPlan installPlan;
		if (!MarkNecessaryFiles(targetPath, installPlan)) {
			GPRINT_ERROR(LogSource::Editor, "Unable to copy all necessary files - build failed.");
			{
				std::scoped_lock scopedLock(processStatus->stringMutex);
				processStatus->stageText = "Build Failed";
				processStatus->detailText = "";
				processStatus->progress = 1.0f;
			}

			return;
		}

		if (!CommitCopies(processStatus, installPlan)) {
			GPRINT_ERROR(LogSource::Editor, "Unable to copy all necessary files - build failed.");
			
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Build Failed";
			processStatus->detailText = "";
			processStatus->progress = 1.0f;

			return;
		}

		if (!BuildPluginManifest(targetPath)) {
			GPRINT_ERROR(LogSource::Editor, "Unable to copy all necessary files - build failed.");

			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Build Failed";
			processStatus->detailText = "";
			processStatus->progress = 1.0f;

			return;
		}

		float compilingAssetArchivesProgress = processStatus->progress;

		{
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Compiling Asset Archives";
		}

		const std::filesystem::path targetCompiledAssetsPath = targetPath / "archives";
		Grindstone::Assets::AssetPackSerializer::SerializeAllAssets(targetCompiledAssetsPath, processStatus, compilingAssetArchivesProgress, 1.0f - compilingAssetArchivesProgress);

		{
			std::scoped_lock scopedLock(processStatus->stringMutex);
			processStatus->stageText = "Build Successful";
			processStatus->detailText = "";
			processStatus->progress = 1.0f;
		}
	}
}
