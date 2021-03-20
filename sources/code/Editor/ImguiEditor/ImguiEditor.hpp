#pragma once

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel;
			class SystemPanel;

			class ImguiEditor {
			public:
				ImguiEditor(EngineCore* engineCore);
				void update();
				void render();
			private:
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				SystemPanel* systemPanel = nullptr;
			};
		}
	}
}
