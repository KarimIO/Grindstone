#pragma once

#include <atomic>
#include <thread>

namespace Grindstone {
	class EngineCore;

	namespace Editor::ImguiEditor {
		class ImguiEditor;
		class Menubar {
		public:
			Menubar(ImguiEditor* editor);
			void Render();
			void RegisterMenuItem(const char* menuItem, void(*fn)(), const char* shortcut);
			void DeregisterMenuItem(const char* menuItem);

			struct MenuNode {
				std::string name;
				std::string shortcut;
				void (*fnPtr)();
				std::vector<MenuNode> children;
			};

		private:
			void RenderFileMenu();
			void RenderEditMenu();
			void RenderViewMenu();
		private:
			void OnNewFile();
			void OnSaveFile();
			void OnSaveAsFile();
			void OnReloadFile();
			void OnLoadFile();
			void OnBuild();
			void OnImportFile();
			void OnUserSettings();
			void OnProjectSettings();
			void OnExit();

			void SaveFile(const std::filesystem::path& path);
			ImguiEditor* editor = nullptr;

			// In the future, this should be a tree
			std::vector<MenuNode> menuItems;
		};
	}
}
