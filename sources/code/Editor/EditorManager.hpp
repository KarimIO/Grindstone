#pragma once

#include "pch.hpp"

#include <fmt/format.h>
#include <Common/Logging.hpp>
#include <Common/Utilities/ModuleLoading.hpp>

#include "EngineCore/EngineCore.hpp"
#include "Commands/CommandList.hpp"
#include "Importers/ImporterManager.hpp"
#include "ScriptBuilder/CSharpBuildManager.hpp"
#include "AssetTemplateRegistry.hpp"
#include "FileManager.hpp"
#include "AssetRegistry.hpp"
#include "GitManager.hpp"
#include "Selection.hpp"
#include "TaskSystem.hpp"

namespace Grindstone {
	namespace Events {
		struct BaseEvent;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
		}

		class IEditor;

		enum class ManipulationMode {
			Translate,
			Rotate,
			Scale
		};

		enum class PlayMode {
			Editor,
			Play,
			Pause
		};

		class Manager {
		public:
			Manager() = default;
			static Manager& Manager::GetInstance();
			Importers::ImporterManager& GetImporterManager();
			AssetRegistry& GetAssetRegistry();
			CommandList& GetCommandList();
			GitManager& GetGitManager();
			Selection& GetSelection();
			TaskSystem& GetTaskSystem();
			AssetTemplateRegistry& GetAssetTemplateRegistry();
			ScriptBuilder::CSharpBuildManager& GetCSharpBuildManager();
			static FileManager& GetFileManager();
			static EngineCore& GetEngineCore();
			bool Initialize(std::filesystem::path projectPath);
			void InitializeQuitCommands();
			~Manager();
			void Run();
			void SetPlayMode(PlayMode newPlayMode);
			PlayMode GetPlayMode() const;
			std::filesystem::path GetProjectPath();
			std::filesystem::path GetAssetsPath();
			std::filesystem::path GetCompiledAssetsPath();
			std::filesystem::path GetEngineBinariesPath();
			bool OnKeyPress(Grindstone::Events::BaseEvent* ev);
			bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
			bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
			ManipulationMode manipulationMode = ManipulationMode::Translate;
			bool isManipulatingInWorldSpace = false;
		private:
			bool LoadEngine();
			bool SetupImguiEditor();
		private:
			std::filesystem::path projectPath;
			std::filesystem::path assetsPath;
			std::filesystem::path compiledAssetsPath;
			std::filesystem::path engineBinariesPath;
			bool shouldClose = false;
			EngineCore* engineCore = nullptr;
			ImguiEditor::ImguiEditor* imguiEditor = nullptr;
			ScriptBuilder::CSharpBuildManager csharpBuildManager;
			CommandList commandList;
			PlayMode playMode;
			Selection selection;
			FileManager projectAssetFileManager;
			FileManager editorAssetFileManager;
			TaskSystem taskSystem;
			AssetRegistry assetRegistry;
			GitManager gitManager;
			AssetTemplateRegistry assetTemplateRegistry;
			Grindstone::Utilities::Modules::Handle engineCoreLibraryHandle;
			Importers::ImporterManager importerManager;
		};
	}
}
