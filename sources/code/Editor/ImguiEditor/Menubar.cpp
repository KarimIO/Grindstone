#include <iostream>
#include <imgui.h>
#include "ImguiEditor.hpp"
#include "../EditorManager.hpp"
#include "Menubar.hpp"

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Manager.hpp"
using namespace Grindstone::Editor::ImguiEditor;

Menubar::Menubar(ImguiEditor* editor) : editor(editor) {}

void Menubar::Render() {
	if (!ImGui::BeginMenuBar()) {
		return;
	}

	if (ImGui::BeginMenu("File")) {
		RenderFileMenu();
	}

	if (ImGui::BeginMenu("Edit")) {
		RenderEditMenu();
	}

	if (ImGui::BeginMenu("View")) {
		RenderViewMenu();
	}

	if (ImGui::BeginMenu("Convert")) {
		RenderConvertMenu();
	}

	ImGui::EndMenuBar();
}

void Menubar::RenderFileMenu() {
	auto& engineCore = Editor::Manager::GetEngineCore();

	if (ImGui::MenuItem("New", "Ctrl+N", false)) {}
	if (ImGui::MenuItem("Save", "Ctrl+S", false)) {
		auto* sceneManager = engineCore.GetSceneManager();
		if (sceneManager->scenes.empty()) {
			Editor::Manager::Print(Grindstone::LogSeverity::Error, "No active scenes.");
		}
		else {
			SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;
			sceneManager->SaveScene(scene->GetPath(), scene);
		}
	}
	if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Reload", "", false)) {}
	if (ImGui::MenuItem("Load...", "Ctrl+O", false)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Import...", "Ctrl+I", false)) {
		editor->ImportFile();
	}
	if (ImGui::MenuItem("Exit", false)) {}
	ImGui::EndMenu();
}

void Menubar::RenderEditMenu() {
	auto& commandList = Editor::Manager::GetInstance().GetCommandList();
	if (ImGui::MenuItem("Undo", "", false, commandList.HasAvailableUndo())) {
		commandList.Undo();
	}
	if (ImGui::MenuItem("Redo", "", false, commandList.HasAvailableRedo())) {
		commandList.Redo();
	}
	ImGui::EndMenu();
}

void Menubar::RenderViewMenu() {
	if (ImGui::MenuItem("Show Asset Browser", "", false)) {}
	if (ImGui::MenuItem("Show Scene Graph", "", false)) {}
	if (ImGui::MenuItem("Show Inspector Panel", "", false)) {}
	if (ImGui::MenuItem("Add Viewport Panel", "", false)) {}
	ImGui::EndMenu();
}

void Menubar::RenderConvertMenu() {
	if (ImGui::MenuItem("Convert Model", "", false)) {
		editor->ShowModelModal();
	}
	if (ImGui::MenuItem("Convert Image", "", false)) {
		editor->ShowImageModal();
	}
	ImGui::EndMenu();
}
