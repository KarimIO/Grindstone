#include <imgui.h>
#include "BaseSettingsPage.hpp"
#include "SettingsWindow.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

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
	ImGuiID frameTitle = ImGui::GetID("#SettingsWindowMainSection");

	if (pages.size() == 0) {
		ImGui::BeginChild(frameTitle, ImVec2{0,  0}, ImGuiChildFlags_None);
		ImGui::Text("No valid settings page.");
		ImGui::EndChild();
		return;
	}

	const float saveFrameHeight = 36.0f;
	float availHeight = ImGui::GetContentRegionAvail().y;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

	ImGui::BeginChild(frameTitle, ImVec2{ 0, availHeight - saveFrameHeight }, ImGuiChildFlags_None);

	ImGui::PopStyleVar();

	if (settingIndex < 0 || settingIndex >= pages.size()) {
		settingIndex = 0;
	}

	Grindstone::UniquePtr<Grindstone::Editor::ImguiEditor::Settings::BasePage>& page = pages[settingIndex].page;
	page->Render();

	ImGui::EndChild();

	ImGuiID saveFrameTitle = ImGui::GetID("#SettingsWindowSaveFrame");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::BeginChild(saveFrameTitle, ImVec2{ 0, saveFrameHeight }, ImGuiWindowFlags_NoScrollbar | ImGuiChildFlags_FrameStyle);
	ImGui::PopStyleVar();

	float totalAvailWidth = ImGui::GetContentRegionAvail().x;
	const float buttonsWidth = 80.0f;
	const float buttonsSpacing = totalAvailWidth - buttonsWidth;
	ImGui::SameLine(buttonsSpacing);

	bool isSaved = ImGui::Button("Save##SettingsButton");
	ImGui::SameLine();
	bool isReset = ImGui::Button("Reset##SettingsButton");

	if (isSaved) {
		page->Save();
	}

	if (isReset) {
		page->Reset();
	}

	ImGui::EndChild();
}

bool SettingsWindow::IsOpen() const {
	return isOpen;
}
