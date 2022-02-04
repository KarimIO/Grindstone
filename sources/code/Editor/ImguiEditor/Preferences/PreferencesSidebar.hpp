#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Preferences {
				class PreferencesWindow;

				class Sidebar {
				public:
					Sidebar(PreferencesWindow* projectSettingsWindow);
					void Render();
				private:
					PreferencesWindow* projectSettingsWindow = nullptr;
				};
			}
		}
	}
}
