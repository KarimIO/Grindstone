#pragma once

#include <vector>
#include "PreferencesPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Preferences {
				class Sidebar;
				class BasePage;

				class PreferencesWindow {
				public:
					PreferencesWindow();
					void Open();
					void OpenPage(PreferencesPage preferencesPage);
					void Render();
					void RenderPreferencesPage();
				private:
					bool isOpen = false;
					int preferenceIndex = 0;
					Sidebar* preferencesSidebar;
					std::vector<BasePage*> pages;
				};
			}
		}
	}
}
