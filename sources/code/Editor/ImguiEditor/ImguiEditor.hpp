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
			class ConsolePanel;
			class SystemPanel;
			class Menubar;
			class ImguiInput;

			class ImguiEditor {
			public:
				ImguiEditor(EngineCore* engineCore);
				void Update();
				void Render();
				void ShowModelModal();
				void ShowImageModal();
				void ImportFile(const char* folderPathToImportTo = "");
			private:
				void RenderDockspace();
			private:
				EngineCore* engineCore = nullptr;
				ImguiInput* input = nullptr;
				ImageConverterModal* imageConverterModal = nullptr;
				ModelConverterModal* modelConverterModal = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				InspectorPanel* inspectorPanel = nullptr;
				AssetBrowserPanel* assetBrowserPanel = nullptr;
				ViewportPanel* viewportPanel = nullptr;
				ConsolePanel* consolePanel = nullptr;
				SystemPanel* systemPanel = nullptr;
				Menubar* menubar = nullptr;
			};
		}
	}
}
