#pragma once

namespace Grindstone {
	class EngineCore;

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
			class Menubar {
			public:
				Menubar(ImguiEditor* editor);
				void Render();
			private:
				void RenderFileMenu();
				void RenderEditMenu();
				void RenderViewMenu();
				void RenderConvertMenu();
			private:
				void OnNewFile();
				void OnSaveFile();
				void OnSaveAsFile();
				void OnReloadFile();
				void OnLoadFile();
				void OnBuild();
				void OnImportFile();
				void OnProjectSettings();
				void OnExit();

				void SaveFile(const char* path);
				ImguiEditor* editor = nullptr;
			};
		}
	}
}
