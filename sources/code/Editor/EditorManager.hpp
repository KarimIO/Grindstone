#pragma once

#include "Common/Logging.hpp"

#include "Commands/CommandList.hpp"
#include "Importers/ImporterManager.hpp"
#include "ScriptBuilder/CSharpBuildManager.hpp"
#include "FileManager.hpp"
#include "Selection.hpp"

namespace Grindstone {
	class EngineCore;

	namespace Events {
		struct BaseEvent;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
		}

		class IEditor;

		class Manager {
		public:
			Manager() = default;
			static Manager& GetInstance();
			Importers::ImporterManager& GetImporterManager();
			CommandList& GetCommandList();
			Selection& GetSelection();
			ScriptBuilder::CSharpBuildManager& GetCSharpBuildManager();
			static FileManager& GetFileManager();
			static EngineCore& GetEngineCore();
			bool Initialize(const char* projectPath);
			void InitializeQuitCommands();
			~Manager();
			void Run();
			std::filesystem::path GetProjectPath();
			std::filesystem::path GetAssetsPath();
			bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
			bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
			static void Print(LogSeverity logSeverity, const char* msg, ...);
		private:
			bool LoadEngine();
			bool SetupImguiEditor();
		private:
			std::filesystem::path projectPath;
			std::filesystem::path assetsPath;
			bool shouldClose = false;
			EngineCore* engineCore = nullptr;
			ImguiEditor::ImguiEditor* imguiEditor = nullptr;
			ScriptBuilder::CSharpBuildManager csharpBuildManager;
			CommandList commandList;
			Selection selection;
			FileManager fileManager;
			Importers::ImporterManager importerManager;
		};
	}
}
