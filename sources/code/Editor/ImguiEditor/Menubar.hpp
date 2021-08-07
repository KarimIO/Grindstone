#pragma once

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
			class Menubar {
			public:
				Menubar(ImguiEditor* editor);
				void render();
			private:
				void renderFileMenu();
				void renderEditMenu();
				void renderViewMenu();
				void renderConvertMenu();
			private:
				ImguiEditor* editor;
			};
		}
	}
}
