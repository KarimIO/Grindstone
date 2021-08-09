#pragma once

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
			class Menubar {
			public:
				Menubar(ImguiEditor* editor);
				void Render();
			private:
				void RenderFileMenu();
				void RenderEditMenu();
				void RenderViewMenu();
				void RenderConvertMenu();
			private:
				ImguiEditor* editor;
			};
		}
	}
}
