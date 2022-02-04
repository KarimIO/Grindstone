#include <imgui.h>
#include "BasePreferencesPage.hpp"
#include "PreferencesWindow.hpp"
#include "PreferencesSidebar.hpp"
#include "Build.hpp"
#include "Plugins.hpp"
using namespace Grindstone::Editor::ImguiEditor::Preferences;

PreferencesWindow::PreferencesWindow() {
	preferencesSidebar = new Sidebar(this);

	pages.push_back(new Build());
	pages.push_back(new Plugins());
}

void PreferencesWindow::Open() {
	isOpen = true;
}

void PreferencesWindow::OpenPage(PreferencesPage preferencesPage) {
	int preferencesPageIndex = (int)preferencesPage;
	if (preferencesPageIndex < 0 || preferencesPageIndex >= pages.size()) {
		return;
	}

	preferenceIndex = preferencesPageIndex;
	pages[preferenceIndex]->Open();
}

void PreferencesWindow::Render() {
	if (!isOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

	if (ImGui::Begin("Preferences", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar)) {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

		if (ImGui::BeginTable("preferencesSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
			ImGui::TableNextColumn();
			preferencesSidebar->Render();
			ImGui::TableNextColumn();
			RenderPreferencesPage();

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();
		ImGui::End();
	}

	ImGui::PopStyleVar();
}

void PreferencesWindow::RenderPreferencesPage() {
	auto assetTopBar = ImGui::GetID("#preferencesWindow");
	ImGui::BeginChildFrame(assetTopBar, ImVec2{}, ImGuiWindowFlags_NoBackground);
	float availWidth = ImGui::GetContentRegionAvailWidth();
	ImVec2 size = ImVec2{ availWidth , 0 };

	if (preferenceIndex < 0 || preferenceIndex >= pages.size()) {
		if (pages.size() == 0) {
			ImGui::Text("No valid preferences file");
			ImGui::EndChildFrame();
		}
		else {
			preferenceIndex = 0;
		}
	}

	pages[preferenceIndex]->Render();

	ImGui::EndChildFrame();
}
