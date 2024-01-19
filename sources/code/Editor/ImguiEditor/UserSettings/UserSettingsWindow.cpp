#include <imgui.h>
#include "UserSettingsWindow.hpp"
#include "CodeTools.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

UserSettingsWindow::UserSettingsWindow() {
	settingsTitle = "User Settings";
	pages.push_back({ "Code Tools", new CodeTools() });
	OpenPage(0);
}
