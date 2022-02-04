#include <imgui.h>
#include "PreferencesSidebar.hpp"
#include "PreferencesWindow.hpp"
using namespace Grindstone::Editor::ImguiEditor::Preferences;

Sidebar::Sidebar(PreferencesWindow* projectSettingsWindow) : projectSettingsWindow(projectSettingsWindow) {}

void Sidebar::Render() {
	auto assetTopBar = ImGui::GetID("#preferencesSidebar");
	ImGui::BeginChildFrame(assetTopBar, ImVec2(), ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvailWidth();
	ImVec2 size = ImVec2{ availWidth , 0 };

	if (ImGui::Button("Build Settings", size)) {
		projectSettingsWindow->OpenPage(PreferencesPage::Build);
	}

	if (ImGui::Button("Plugins", size)) {
		projectSettingsWindow->OpenPage(PreferencesPage::Plugins);
	}

	ImGui::EndChildFrame();
}
