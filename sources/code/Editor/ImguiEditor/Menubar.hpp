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
		};
	}
}
