#include <iostream>
#include <imgui/imgui.h>
#include "Menubar.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Menubar::render() {
	if (!ImGui::BeginMenuBar()) {
		return;
	}

	if (ImGui::BeginMenu("File")) {
		renderFileMenu();
	}

	if (ImGui::BeginMenu("View")) {
		renderViewMenu();
	}

	ImGui::EndMenuBar();
}

void Menubar::renderFileMenu() {
	if (ImGui::MenuItem("New", "", false)) {}
	if (ImGui::MenuItem("Save", "", false)) {}
	if (ImGui::MenuItem("Save As", "", false)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Import", "", false)) {}
	if (ImGui::MenuItem("Load", "", false)) {}
	if (ImGui::MenuItem("Load From", "", false)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Close", "", false)) {}
	ImGui::EndMenu();
}

void Menubar::renderViewMenu() {
	if (ImGui::MenuItem("Show Asset Browser", "", false)) {}
	if (ImGui::MenuItem("Show Scene Graph", "", false)) {}
	if (ImGui::MenuItem("Show Inspector Panel", "", false)) {}
	if (ImGui::MenuItem("Add Viewport Panel", "", false)) {}
	ImGui::EndMenu();
}
