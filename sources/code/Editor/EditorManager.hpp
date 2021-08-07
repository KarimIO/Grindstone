#pragma once

#include "Commands/CommandList.hpp"

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
			CommandList& getCommandList();
			bool initialize();
			~Manager();
			void run();
		private:
			bool loadEngine();
			bool setupImguiEditor();
		private:
			EngineCore* engineCore;
			ImguiEditor::ImguiEditor* imguiEditor;
			CommandList commandList;
		};
	}
}