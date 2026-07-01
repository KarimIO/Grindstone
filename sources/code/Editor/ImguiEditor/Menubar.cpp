#include <iostream>
#include <ranges>
#include <imgui.h>

#include <Common/Window/WindowManager.hpp>
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

	if (!menuItems.empty()) {
		if (ImGui::BeginMenu("Custom Commands")) {
			for (Menubar::MenubarItem& item : menuItems) {
				if (ImGui::MenuItem(item.text.c_str(), item.shortcut.c_str(), false)) {
					item.fnPtr();
				}
			}
			ImGui::EndMenu();
		}
	}

	ImGui::EndMenuBar();
}

void Menubar::RenderFileMenu() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();

	if (ImGui::MenuItem("New", "Ctrl+N", false)) {
		OnNewFile();
	}
	if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false)) {
		OnSaveAsFile();
	}
	ImGui::Separator();
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
	// TODO: New Prefab
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
	// TODO: Reload Prefab
}

void Menubar::OnLoadFile() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	WindowManager* windowManager = engineCore.windowManager;
	Window* window = windowManager->GetWindowByIndex(0);
	std::filesystem::path filePath = window->OpenFileDialogue("Scene File (.gscene)\0*.gscene\0");

	Grindstone::Uuid uuid;
	std::string filePathStr = filePath.string();
	if (!filePath.empty() && Grindstone::Uuid::MakeFromString(filePathStr.c_str(), uuid)) {
		// TODO: Load Prefab
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
	// TODO: Save Prefab
}

void Menubar::RegisterMenuItem(const char* menuItem, void(*fn)(), const char* shortcut) {
	menuItems.emplace_back(Menubar::MenubarItem{ .text = menuItem, .shortcut = shortcut == nullptr ? "" : shortcut, .fnPtr = fn});
}

void Menubar::DeregisterMenuItem(const char* menuItem) {
	for (auto it = menuItems.rbegin(); it < menuItems.rend(); it++) {
		if (it->text == menuItem) {
			menuItems.erase((it + 1).base());
			break;
		}
	}
}
