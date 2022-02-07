#include <iostream>
#include <imgui.h>
#include "ImguiEditor.hpp"

#include "UserSettings/UserSettingsWindow.hpp"
#include "ProjectSettings/ProjectSettingsWindow.hpp"
#include "Common/Window/WindowManager.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/EngineCore.hpp"

#include "../EditorManager.hpp"
#include "Menubar.hpp"
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
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;
	bool doesSceneHavePath = scene && strlen(scene->GetPath());

	if (ImGui::MenuItem("New", "Ctrl+N", false)) {
		OnNewFile();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S", false, doesSceneHavePath)) {
		OnSaveFile();
	}
	if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false)) {
		OnSaveAsFile();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Reload", "", false, doesSceneHavePath)) {
		OnReloadFile();
	}
	if (ImGui::MenuItem("Load...", "Ctrl+O", false)) {
		OnLoadFile();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Build...", false)) {
		OnBuild();
	}
	if (ImGui::MenuItem("Import...", "Ctrl+I", false)) {
		OnImportFile();
	}
	if (ImGui::MenuItem("User Settings...", "Ctrl+Shift+P", false)) {
		OnUserSettings();
	}
	if (ImGui::MenuItem("Project Settings...", "Ctrl+P", false)) {
		OnProjectSettings();
	}
	if (ImGui::MenuItem("Exit", false)) {
		OnExit();
	}
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

void Menubar::OnNewFile() {
	auto* sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();
	sceneManager->AddEmptyScene("Untitled Scene");
}

void Menubar::OnSaveFile() {
	SaveFile("");
}

void Menubar::OnSaveAsFile() {
	auto windowManager = Editor::Manager::GetEngineCore().windowManager;
	auto window = windowManager->GetWindowByIndex(0);
	auto filePath = window->SaveFileDialogue("Scene File (.json)\0*.json\0");

	if (!filePath.empty()) {
		SaveFile(filePath.c_str());
	}
}

void Menubar::OnReloadFile() {
	auto* sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();
	sceneManager->LoadScene(sceneManager->scenes.begin()->second->GetPath());
}

void Menubar::OnLoadFile() {
	auto& engineCore = Editor::Manager::GetEngineCore();
	auto windowManager = engineCore.windowManager;
	auto window = windowManager->GetWindowByIndex(0);
	auto filePath = window->OpenFileDialogue("Scene File (.json)\0*.json\0");

	if (!filePath.empty()) {
		auto* sceneManager = engineCore.GetSceneManager();
		sceneManager->LoadScene(filePath.c_str());
	}
}

void Menubar::OnBuild() {
	editor->StartBuild();
}

void Menubar::OnImportFile() {
	editor->ImportFile();
}

void Menubar::OnUserSettings() {
	editor->userSettingsWindow->Open();
}

void Menubar::OnProjectSettings() {
	editor->projectSettingsWindow->Open();
}

void Menubar::OnExit() {
	Editor::Manager::GetInstance().OnTryQuit(nullptr);
}

void Menubar::SaveFile(const char* path = "") {
	auto* sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();
	if (sceneManager->scenes.empty()) {
		Editor::Manager::Print(Grindstone::LogSeverity::Error, "No active scenes.");
	}
	else {
		SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;
		if (strlen(path) == 0) {
			sceneManager->SaveScene(scene->GetPath(), scene);
		}
		else {
			sceneManager->SaveScene(path, scene);
		}
	}
}
