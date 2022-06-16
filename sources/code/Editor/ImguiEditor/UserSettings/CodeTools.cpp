#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/ImguiEditor/Components/ListEditor.hpp"

#include "Editor/EditorManager.hpp"

#include "CodeTools.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Settings::CodeTools::Open() {
	std::filesystem::path settingsFile = Editor::Manager::GetInstance().GetProjectPath() / "userSettings/codeToolsPath.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	auto settingsPath = settingsFile.string();
	msBuildPath = Utils::LoadFileText(settingsPath.c_str());
}

void Settings::CodeTools::Render() {
	ImGui::Text("Build Options");
	ImGui::Separator();

	ImGui::InputText("MsBuild Path", &msBuildPath);
	if (ImGui::Button("Save")) {
		WriteFile();
	}
}

void Settings::CodeTools::WriteFile() {
	std::filesystem::path settingsFile = Editor::Manager::GetInstance().GetProjectPath() / "userSettings/codeToolsPath.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	auto settingsPath = settingsFile.string();
	std::ofstream outputFile(settingsPath.c_str());

	if (!outputFile.is_open()) {
		return;
	}

	outputFile.clear();
	outputFile << msBuildPath;
	outputFile.close();
}
