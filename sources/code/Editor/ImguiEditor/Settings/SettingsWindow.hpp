#pragma once

#include <vector>
#include <string>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class BasePage;

				class SettingsWindow {
				public:
					void Open();
					void OpenPage(int preferencesPage);
					void Render();
					void RenderSettingsPage();
					virtual void RenderSideBar() = 0;
				protected:
					bool isOpen = false;
					int preferenceIndex = 0;
					std::string settingsTitle;
					std::vector<BasePage*> pages;
				};
			}
		}
	}
}
