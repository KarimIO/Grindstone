#pragma once

#include <vector>
#include "../Settings/SettingsWindow.hpp"
#include "UserSettingsPage.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class Sidebar;
				class BasePage;

				class UserSettingsWindow : public SettingsWindow {
				public:
					UserSettingsWindow();
					void OpenPage(UserSettingsPage preferencesPage);
					virtual void RenderSideBar() override;
				};
			}
		}
	}
}
