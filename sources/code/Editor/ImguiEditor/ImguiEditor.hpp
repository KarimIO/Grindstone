#pragma once

#include <vector>
#include <entt/entt.hpp>
#include <vulkan/vulkan.h>
#include <imgui.h>

#include <Common/Event/BaseEvent.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Editor/ImguiEditor/AssetPicker.hpp>
#include "imgui_markdown.h"

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			enum class FontType {
				Regular,
				Bold,
				Italic,
				H1,
				H2,
				H3,
				Count
			};

			class ImguiRenderer;

			namespace Settings {
				class UserSettingsWindow;
				class ProjectSettingsWindow;
			}

			class PluginsWindow;
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
				void CreateWindows();
				void Update();
				void Render();
				void PromptAssetPicker(AssetType assetType, AssetPicker::AssetPickerCallback callback);
				void StartBuild();
				void ImportFile(const char* folderPathToImportTo = "");
				ViewportPanel* GetViewportPanel();
				ImFont* GetFont(FontType type) const;
				const ImGui::MarkdownConfig& GetMarkdownConfig() const;
			private:
				void RenderDockspace();
				void SetupFonts();
				void SetupStyles();
				void SetupColors();
				bool OnWindowResize(Events::BaseEvent* ev);
				void PerformResize();
			private:
				bool queueResize = false;

				std::array<ImFont*, static_cast<size_t>(FontType::Count)> fonts;
				ImGui::MarkdownConfig markdownConfig;
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
				PluginsWindow* pluginsWindow = nullptr;

				std::string imguiIniFile;
				std::string imguiLogFile;
			};
		}
	}
}
