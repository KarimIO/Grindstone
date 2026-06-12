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

			struct MenubarItem {
				std::string text; // Eventually we should localize this
				std::string shortcut;
				void (*fnPtr)();
			};

			// In the future, this should be a tree
			std::vector<MenubarItem> menuItems;
		};
	}
}
