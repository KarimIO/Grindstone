#include <iostream>
#include <imgui/imgui.h>
#include "ImguiEditor.hpp"
#include "../EditorManager.hpp"
#include "Menubar.hpp"
using namespace Grindstone::Editor::ImguiEditor;

Menubar::Menubar(ImguiEditor* editor) : editor(editor) {}

void Menubar::render() {
	if (!ImGui::BeginMenuBar()) {
		return;
	}

	if (ImGui::BeginMenu("File")) {
		renderFileMenu();
	}

	if (ImGui::BeginMenu("Edit")) {
		renderEditMenu();
	}

	if (ImGui::BeginMenu("View")) {
		renderViewMenu();
	}

	if (ImGui::BeginMenu("Convert")) {
		renderConvertMenu();
	}

	ImGui::EndMenuBar();
}

void Menubar::renderFileMenu() {
	if (ImGui::MenuItem("New", "Ctrl+N", false)) {}
	if (ImGui::MenuItem("Save", "Ctrl+S", false)) {}
	if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Reload", "", false)) {}
	if (ImGui::MenuItem("Load...", "Ctrl+O", false)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Import...", "Ctrl+I", false)) {
		editor->importFile();
	}
	if (ImGui::MenuItem("Exit", false)) {}
	ImGui::EndMenu();
}

void Menubar::renderEditMenu() {
	auto& commandList = Editor::Manager::GetInstance().getCommandList();
	if (ImGui::MenuItem("Undo", "", false, commandList.HasAvailableUndo())) {
		commandList.Undo();
	}
	if (ImGui::MenuItem("Redo", "", false, commandList.HasAvailableRedo())) {
		commandList.Redo();
	}
	ImGui::EndMenu();
}

void Menubar::renderViewMenu() {
	if (ImGui::MenuItem("Show Asset Browser", "", false)) {}
	if (ImGui::MenuItem("Show Scene Graph", "", false)) {}
	if (ImGui::MenuItem("Show Inspector Panel", "", false)) {}
	if (ImGui::MenuItem("Add Viewport Panel", "", false)) {}
	ImGui::EndMenu();
}

void Menubar::renderConvertMenu() {
	if (ImGui::MenuItem("Convert Model", "", false)) {
		editor->showModelModal();
	}
	if (ImGui::MenuItem("Convert Image", "", false)) {
		editor->showImageModal();
	}
	ImGui::EndMenu();
}
