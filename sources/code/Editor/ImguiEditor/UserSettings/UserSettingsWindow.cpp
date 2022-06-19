#include <imgui.h>
#include "UserSettingsWindow.hpp"
#include "CodeTools.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

UserSettingsWindow::UserSettingsWindow() {
	settingsTitle = "User Settings";
	pages.push_back(new CodeTools());
}

void UserSettingsWindow::OpenPage(UserSettingsPage preferencesPage) {
	int preferencesPageIndex = (int)preferencesPage;
	if (preferencesPageIndex < 0 || preferencesPageIndex >= pages.size()) {
		return;
	}

	preferenceIndex = preferencesPageIndex;
	pages[preferenceIndex]->Open();
}

void UserSettingsWindow::RenderSideBar() {
	auto assetTopBar = ImGui::GetID("#UserSettingsSidebar");
	ImGui::BeginChildFrame(assetTopBar, ImVec2(), ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvail().x;
	ImVec2 size = ImVec2{ availWidth , 0 };

	if (ImGui::Button("Code Tools", size)) {
		OpenPage(UserSettingsPage::CodeTools);
	}

	ImGui::EndChildFrame();
}
