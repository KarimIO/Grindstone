#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class SceneHeirarchyPanel;
			class ModelConverterModal;
			class ImageConverterModal;
			class AssetBrowserPanel;
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
				void deselectFromInspector();
				void selectFile(std::string selectedFileType, std::string selectedFilePath);
				void selectEntity(entt::entity selectedEntity);
				void importFile(const char* folderPathToImportTo = "");
			private:
				void renderDockspace();
			private:
				EngineCore* engineCore = nullptr;
				entt::entity selectedEntity = entt::null;
				ImguiInput* input = nullptr;
				ImageConverterModal* imageConverterModal = nullptr;
				ModelConverterModal* modelConverterModal = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				InspectorPanel* inspectorPanel = nullptr;
				AssetBrowserPanel* assetBrowserPanel = nullptr;
				ViewportPanel* viewportPanel = nullptr;
				SystemPanel* systemPanel = nullptr;
				Menubar* menubar = nullptr;
			};
		}
	}
}
