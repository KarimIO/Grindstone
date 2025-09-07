#include <iostream>
#include <imgui.h>

#include <Common/Window/WindowManager.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Editor/AssetPackSerializer.hpp>
#include <Editor/EditorManager.hpp>

#include "ImguiEditor.hpp"
#include "UserSettings/UserSettingsWindow.hpp"
#include "ProjectSettings/ProjectSettingsWindow.hpp"
#include "PluginsWindow.hpp"
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

	ImGui::EndMenuBar();
}

void Menubar::RenderFileMenu() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;
	bool doesSceneHavePath = scene && scene->HasPath();

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
	if (ImGui::MenuItem("Reload", nullptr, false, doesSceneHavePath)) {
		OnReloadFile();
	}
	if (ImGui::MenuItem("Load...", "Ctrl+O", false)) {
		OnLoadFile();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Build...", nullptr, false)) {
		OnBuild();
	}
	if (ImGui::MenuItem("Import...", "Ctrl+I", false)) {
		OnImportFile();
	}
	if (ImGui::MenuItem("User Settings...", "Ctrl+Shift+P", editor->userSettingsWindow->IsOpen())) {
		OnUserSettings();
	}
	if (ImGui::MenuItem("Project Settings...", "Ctrl+P", editor->projectSettingsWindow->IsOpen())) {
		OnProjectSettings();
	}
	if (ImGui::MenuItem("Plugins Settings...", "Ctrl+Shift+E", editor->pluginsWindow->IsOpen())) {
		editor->pluginsWindow->Open();
	}
	if (ImGui::MenuItem("Exit", nullptr, false)) {
		OnExit();
	}
	ImGui::EndMenu();
}

void Menubar::RenderEditMenu() {
	auto& commandList = Editor::Manager::GetInstance().GetCommandList();
	if (ImGui::MenuItem("Undo", nullptr, false, commandList.HasAvailableUndo())) {
		commandList.Undo();
	}
	if (ImGui::MenuItem("Redo", nullptr, false, commandList.HasAvailableRedo())) {
		commandList.Redo();
	}
	ImGui::EndMenu();
}

void Menubar::RenderViewMenu() {
	if (ImGui::MenuItem("Show Asset Browser", nullptr, false)) {}
	if (ImGui::MenuItem("Show Scene Graph", nullptr, false)) {}
	if (ImGui::MenuItem("Show Inspector Panel", nullptr, false)) {}
	if (ImGui::MenuItem("Add Viewport Panel", nullptr, false)) {}
	ImGui::EndMenu();
}

void Menubar::OnNewFile() {
	SceneManagement::SceneManager* sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();
	sceneManager->CreateEmptyScene("Untitled Scene");
}

void Menubar::OnSaveFile() {
	SaveFile("");
}

void Menubar::OnSaveAsFile() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	WindowManager* windowManager = engineCore.windowManager;
	Window* window = windowManager->GetWindowByIndex(0);
	std::filesystem::path filePath = window->SaveFileDialogue("Scene File (.gscene)\0*.gscene\0");

	if (!filePath.empty()) {
		SaveFile(filePath);
	}
}

void Menubar::OnReloadFile() {
	SceneManagement::SceneManager* sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();
	sceneManager->LoadScene(sceneManager->scenes.begin()->first);
}

void Menubar::OnLoadFile() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	WindowManager* windowManager = engineCore.windowManager;
	Window* window = windowManager->GetWindowByIndex(0);
	std::filesystem::path filePath = window->OpenFileDialogue("Scene File (.gscene)\0*.gscene\0");

	Grindstone::Uuid uuid;
	const char* filePathStr = filePath.string().c_str();
	if (!filePath.empty() && Grindstone::Uuid::MakeFromString(filePathStr, uuid)) {
		auto* sceneManager = engineCore.GetSceneManager();
		sceneManager->LoadScene(uuid);
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

void Menubar::SaveFile(const std::filesystem::path& path) {
	auto* sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();
	if (sceneManager->scenes.empty()) {
		GPRINT_ERROR(LogSource::Editor, "No active scenes.");
	}
	else {
		SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;
		if (path.empty()) {
			sceneManager->SaveScene(scene->GetPath(), scene);
		}
		else {
			sceneManager->SaveScene(path, scene);
		}
	}
}
