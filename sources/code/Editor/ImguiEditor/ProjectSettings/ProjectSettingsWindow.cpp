#include <imgui.h>
#include "ProjectSettingsPage.hpp"
#include "ProjectSettingsWindow.hpp"
#include "Build.hpp"
#include "Platforms.hpp"
#include "Plugins.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

ProjectSettingsWindow::ProjectSettingsWindow() {
	settingsTitle = "Project Settings";
	pages.push_back({ "Build", new Build() });
	pages.push_back({ "Platforms", new Platforms() });
	pages.push_back({ "Plugins", new Plugins() });
	OpenPage(0);
}
