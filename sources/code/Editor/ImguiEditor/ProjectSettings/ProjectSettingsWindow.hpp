#pragma once

#include <vector>
#include "ProjectSettingsPage.hpp"
#include "../Settings/SettingsWindow.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			namespace Settings {
				class Sidebar;
				class BasePage;

				class ProjectSettingsWindow : public SettingsWindow {
				public:
					ProjectSettingsWindow();
					void OpenPage(ProjectSettingsPage preferencesPage);
					virtual void RenderSideBar() override;
				};
			}
		}
	}
}
