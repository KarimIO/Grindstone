#pragma once

#include <vector>
#include "ProjectSettingsPage.hpp"
#include "../Settings/SettingsWindow.hpp"

namespace Grindstone::Editor::ImguiEditor::Settings {
	class Sidebar;
	class BasePage;

	class ProjectSettingsWindow : public SettingsWindow {
	public:
		ProjectSettingsWindow();
	};
}
