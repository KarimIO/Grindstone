#pragma once

#include <entt/entt.hpp>

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
				void updateSelectedEntity(entt::entity selectedEntity);
			private:
				void renderDockspace();
			private:
				entt::entity selectedEntity = entt::null;
				ImguiInput* input = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				InspectorPanel* inspectorPanel = nullptr;
				SystemPanel* systemPanel = nullptr;
				Menubar* menubar = nullptr;
			};
		}
	}
}
