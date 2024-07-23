#pragma once

#include <vector>
#include <entt/entt.hpp>
#include <vulkan/vulkan.h>

#include <Common/Event/BaseEvent.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Editor/ImguiEditor/AssetPicker.hpp>

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class ImguiRenderer;

			namespace Settings {
				class UserSettingsWindow;
				class ProjectSettingsWindow;
			}

			class SceneHeirarchyPanel;
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
			class AssetPicker;
			class TracingPanel;

			class ImguiEditor {
			public:
				friend Menubar;

				ImguiEditor(EngineCore* engineCore);
				~ImguiEditor();
				void Update();
				void Render();
				void PromptAssetPicker(AssetType assetType, AssetPicker::AssetPickerCallback callback);
				void StartBuild();
				void ImportFile(const char* folderPathToImportTo = "");
			private:
				void RenderDockspace();
				void SetupFonts();
				void SetupStyles();
				void SetupColors();
				bool OnWindowResize(Events::BaseEvent* ev);
				void PerformResize();
			private:
				bool queueResize = false;
				EngineCore* engineCore = nullptr;
				ImguiInput* input = nullptr;
				SceneHeirarchyPanel* sceneHeirarchyPanel = nullptr;
				AssetBrowserPanel* assetBrowserPanel = nullptr;
				AssetPicker* assetPicker = nullptr;
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
				ImguiRenderer* imguiRenderer = nullptr;
				TracingPanel* tracingPanel = nullptr;

				std::string imguiIniFile;
				std::string imguiLogFile;
			};
		}
	}
}
