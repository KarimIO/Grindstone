#pragma once

#include "Commands/CommandList.hpp"
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
			CommandList& GetCommandList();
			Selection& GetSelection();
			EngineCore& GetEngineCore();
			bool Initialize();
			~Manager();
			void Run();
		private:
			bool LoadEngine();
			bool SetupImguiEditor();
		private:
			EngineCore* engineCore = nullptr;
			ImguiEditor::ImguiEditor* imguiEditor = nullptr;
			CommandList commandList;
			Selection selection;
		};
	}
}