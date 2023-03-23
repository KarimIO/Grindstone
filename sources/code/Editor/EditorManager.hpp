#pragma once

#include <fmt/format.h>
#include "Common/Logging.hpp"

#include "EngineCore/EngineCore.hpp"
#include "Commands/CommandList.hpp"
#include "Importers/ImporterManager.hpp"
#include "ScriptBuilder/CSharpBuildManager.hpp"
#include "FileManager.hpp"
#include "Selection.hpp"

namespace Grindstone {
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
			bool Initialize(std::filesystem::path projectPath);
			void InitializeQuitCommands();
			~Manager();
			void Run();
			std::filesystem::path GetProjectPath();
			std::filesystem::path GetAssetsPath();
			std::filesystem::path GetCompiledAssetsPath();
			std::filesystem::path GetEngineBinariesPath();
			bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
			bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
			template<typename... Args>
			static void Print(LogSeverity logSeverity, fmt::format_string<Args...> fmt, Args &&...args) {
				std::string outStr = fmt::format(fmt, std::forward<Args>(args)...);
				GetInstance().engineCore->Print(logSeverity, outStr.c_str());
			}
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
			Selection selection;
			FileManager fileManager;
			Importers::ImporterManager importerManager;
		};
	}
}
