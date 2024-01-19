#pragma once

#include <vector>
#include "../Settings/SettingsWindow.hpp"
#include "UserSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	class Sidebar;
	class BasePage;

	class UserSettingsWindow : public SettingsWindow {
	public:
		UserSettingsWindow();
	};
}
