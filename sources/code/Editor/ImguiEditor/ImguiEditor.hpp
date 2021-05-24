#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel;
			class ModelConverterModal;
			class ImageConverterModal;
			class InspectorPanel;
			class ViewportPanel;
			class SystemPanel;
			class Menubar;
			class ImguiInput;

			class ImguiEditor {
			public:
				ImguiEditor(EngineCore* engineCore);
				void update();
				void render();
				void showModelModal();
				void showImageModal();
				void updateSelectedEntity(entt::entity selectedEntity);
			private:
				void renderDockspace();
			private:
				entt::entity selectedEntity = entt::null;
				ImguiInput* input = nullptr;
				ImageConverterModal* imageConverterModal = nullptr;
				ModelConverterModal* modelConverterModal = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				InspectorPanel* inspectorPanel = nullptr;
				ViewportPanel* viewportPanel = nullptr;
				SystemPanel* systemPanel = nullptr;
				Menubar* menubar = nullptr;
			};
		}
	}
}
