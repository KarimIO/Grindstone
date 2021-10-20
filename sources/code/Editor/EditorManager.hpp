#pragma once

#include "Common/Logging.hpp"

#include "Commands/CommandList.hpp"
#include "Importers/ImporterManager.hpp"
#include "FileManager.hpp"
#include "Selection.hpp"

namespace Grindstone {
	class EngineCore;

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
			static FileManager& GetFileManager();
			static EngineCore& GetEngineCore();
			bool Initialize();
			~Manager();
			void Run();
			static void Print(LogSeverity logSeverity, const char* msg, ...);
		private:
			bool LoadEngine();
			bool SetupImguiEditor();
		private:
			EngineCore* engineCore = nullptr;
			ImguiEditor::ImguiEditor* imguiEditor = nullptr;
			CommandList commandList;
			Selection selection;
			FileManager fileManager;
			Importers::ImporterManager importerManager;
		};
	}
}