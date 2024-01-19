#include <imgui.h>
#include "BaseSettingsPage.hpp"
#include "SettingsWindow.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

SettingsWindow::~SettingsWindow() {
	for (PageData& page : pages) {
		if (page.page != nullptr) {
			delete page.page;
			page.page = nullptr;
		}
	}
}

void SettingsWindow::Open() {
	isOpen = true;
}

void SettingsWindow::OpenPage(size_t preferencesPage) {
	size_t preferencesPageIndex = (size_t)preferencesPage;
	if (preferencesPageIndex < 0 || preferencesPageIndex >= pages.size()) {
		return;
	}

	settingIndex = preferencesPageIndex;
	pages[settingIndex].page->Open();
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

void SettingsWindow::RenderSideBar() {
	ImGuiID assetTopBar = ImGui::GetID("#SettingsSidebar");
	ImGui::BeginChildFrame(assetTopBar, ImVec2(), ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvail().x;
	ImVec2 size = ImVec2{ availWidth , 0 };

	for (size_t i = 0; i < pages.size(); ++i) {
		PageData& page = pages[i];

		if (page.page == nullptr) {
			continue;
		}

		if (ImGui::Button(page.title.c_str(), size)) {
			OpenPage(i);
		}
	}

	ImGui::EndChildFrame();
}

void SettingsWindow::RenderSettingsPage() {
	ImGuiID assetTopBar = ImGui::GetID("#SettingsWindow");
	ImGui::BeginChildFrame(assetTopBar, ImVec2{}, ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvail().x;
	ImVec2 size = ImVec2{ availWidth , 0 };

	if (settingIndex < 0 || settingIndex >= pages.size()) {
		if (pages.size() == 0) {
			ImGui::Text("No valid settings page.");
			ImGui::EndChildFrame();
		}
		else {
			settingIndex = 0;
		}
	}

	pages[settingIndex].page->Render();

	ImGui::EndChildFrame();
}

bool SettingsWindow::IsOpen() const {
	return isOpen;
}
