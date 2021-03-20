#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel;

			class ImguiEditor {
			public:
				ImguiEditor();
				void update();
				void render();
			private:
				SceneHeirarchyPanel* sceneHeirarchyPanel;
			};
		}
	}
}
