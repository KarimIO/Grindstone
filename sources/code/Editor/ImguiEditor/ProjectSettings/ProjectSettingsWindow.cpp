#include <imgui.h>
#include "ProjectSettingsPage.hpp"
#include "ProjectSettingsWindow.hpp"
#include "Build.hpp"
#include "Platforms.hpp"
#include "Plugins.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

ProjectSettingsWindow::ProjectSettingsWindow() {
	settingsTitle = "Project Settings";
	pages.push_back(new Build());
	pages.push_back(new Platforms());
	pages.push_back(new Plugins());
}

void ProjectSettingsWindow::OpenPage(ProjectSettingsPage preferencesPage) {
	int preferencesPageIndex = (int)preferencesPage;
	if (preferencesPageIndex < 0 || preferencesPageIndex >= pages.size()) {
		return;
	}

	preferenceIndex = preferencesPageIndex;
	pages[preferenceIndex]->Open();
}

void ProjectSettingsWindow::RenderSideBar() {
	auto assetTopBar = ImGui::GetID("#ProjectSettingsSidebar");
	ImGui::BeginChildFrame(assetTopBar, ImVec2(), ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvailWidth();
	ImVec2 size = ImVec2{ availWidth , 0 };

	if (ImGui::Button("Build Settings", size)) {
		OpenPage(ProjectSettingsPage::Build);
	}

	if (ImGui::Button("Platforms", size)) {
		OpenPage(ProjectSettingsPage::Platforms);
	}

	if (ImGui::Button("Plugins", size)) {
		OpenPage(ProjectSettingsPage::Plugins);
	}

	ImGui::EndChildFrame();
}
