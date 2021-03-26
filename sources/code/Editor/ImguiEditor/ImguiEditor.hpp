#pragma once

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel;
			class InspectorPanel;
			class SystemPanel;
			class Menubar;
			class ImguiInput;

			class ImguiEditor {
			public:
				ImguiEditor(EngineCore* engineCore);
				void update();
				void render();
			private:
				void renderDockspace();
			private:
				ImguiInput* input = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				InspectorPanel* inspectorPanel = nullptr;
				SystemPanel* systemPanel = nullptr;
				Menubar* menubar = nullptr;
			};
		}
	}
}
