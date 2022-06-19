#include <imgui.h>
#include "BaseSettingsPage.hpp"
#include "SettingsWindow.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

void SettingsWindow::Open() {
	isOpen = true;
}

void SettingsWindow::OpenPage(int preferencesPage) {
	int preferencesPageIndex = (int)preferencesPage;
	if (preferencesPageIndex < 0 || preferencesPageIndex >= pages.size()) {
		return;
	}

	preferenceIndex = preferencesPageIndex;
	pages[preferenceIndex]->Open();
}

void SettingsWindow::Render() {
	if (!isOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

	if (ImGui::Begin(settingsTitle.c_str(), &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar)) {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

		if (ImGui::BeginTable("SettingsSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
			ImGui::TableNextColumn();
			RenderSideBar();
			ImGui::TableNextColumn();
			RenderSettingsPage();

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();
		ImGui::End();
	}

	ImGui::PopStyleVar();
}

void SettingsWindow::RenderSettingsPage() {
	auto assetTopBar = ImGui::GetID("#SettingsWindow");
	ImGui::BeginChildFrame(assetTopBar, ImVec2{}, ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvail().x;
	ImVec2 size = ImVec2{ availWidth , 0 };

	if (preferenceIndex < 0 || preferenceIndex >= pages.size()) {
		if (pages.size() == 0) {
			ImGui::Text("No valid settings page.");
			ImGui::EndChildFrame();
		}
		else {
			preferenceIndex = 0;
		}
	}

	pages[preferenceIndex]->Render();

	ImGui::EndChildFrame();
}
