#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class UserSettingsWindow;
				class ProjectSettingsWindow;
			}

			class SceneHeirarchyPanel;
			class ModelConverterModal;
			class ImageConverterModal;
			class AssetBrowserPanel;
			class InspectorPanel;
			class ViewportPanel;
			class ConsolePanel;
			class SystemPanel;
			class StatsPanel;
			class BuildPopup;
			class Menubar;
			class ImguiInput;
			class ControlBar;
			class StatusBar;

			class ImguiEditor {
			public:
				friend Menubar;

				ImguiEditor(EngineCore* engineCore);
				~ImguiEditor();
				void Update();
				void Render();
				void ShowModelModal();
				void ShowImageModal();
				void StartBuild();
				void ImportFile(const char* folderPathToImportTo = "");
			private:
				void RenderDockspace();
				void SetupFonts();
				void SetupStyles();
				void SetupColors();
			private:
				EngineCore* engineCore = nullptr;
				ImguiInput* input = nullptr;
				ImageConverterModal* imageConverterModal = nullptr;
				ModelConverterModal* modelConverterModal = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				AssetBrowserPanel* assetBrowserPanel = nullptr;
				Settings::UserSettingsWindow* userSettingsWindow = nullptr;
				Settings::ProjectSettingsWindow* projectSettingsWindow = nullptr;
				InspectorPanel* inspectorPanel = nullptr;
				ViewportPanel* viewportPanel = nullptr;
				ConsolePanel* consolePanel = nullptr;
				SystemPanel* systemPanel = nullptr;
				StatsPanel* statsPanel = nullptr;
				BuildPopup* buildPopup = nullptr;
				ControlBar* controlBar = nullptr;
				StatusBar* statusBar = nullptr;
				Menubar* menubar = nullptr;
			};
		}
	}
}
