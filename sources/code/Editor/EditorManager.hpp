#pragma once

namespace Grindstone {
	class EngineCore;

    namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
		}

        class IEditor;

        class Manager {
        public:
            bool initialize();
            ~Manager();
			void run();
        private:
            bool loadEngine();
			bool setupImguiEditor();
        private:
			EngineCore* engineCore;
            ImguiEditor::ImguiEditor* imguiEditor;
        };
    }
}