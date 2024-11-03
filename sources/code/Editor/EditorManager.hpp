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

namespace Grindstone::Events {
	struct BaseEvent;
}

namespace Grindstone::Editor {
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
		ImguiEditor::ImguiEditor& GetImguiEditor();
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
		// Begin to set PlayMode at end of frame.
		void SetPlayMode(PlayMode newPlayMode);
		PlayMode GetPlayMode() const;
		const std::filesystem::path& GetProjectPath() const;
		const std::filesystem::path& GetAssetsPath() const;
		const std::filesystem::path& GetCompiledAssetsPath() const;
		const std::filesystem::path& GetEngineBinariesPath() const;
		bool OnKeyPress(Grindstone::Events::BaseEvent* ev);
		bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
		bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
		ManipulationMode manipulationMode = ManipulationMode::Translate;
		bool isManipulatingInWorldSpace = false;
	private:
		bool LoadEngine();
		bool SetupImguiEditor();

		// Actually change the PlayMode, to be used only by SetPlayMode.
		void TransferPlayMode(PlayMode newPlayMode);
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
		// Current PlayMode - should we update objects? And how?
		PlayMode playMode;
		// Play Mode that will be set at end of frame
		PlayMode newPlayMode;
		Selection selection;
		FileManager fileManager;
		TaskSystem taskSystem;
		AssetRegistry assetRegistry;
		GitManager gitManager;
		AssetTemplateRegistry assetTemplateRegistry;
		Grindstone::Utilities::Modules::Handle engineCoreLibraryHandle;
		Importers::ImporterManager importerManager;
	};
}
