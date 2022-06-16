#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include "Editor/EditorManager.hpp"
#include "Editor/ImguiEditor/Components/ListEditor.hpp"

#include "Platforms.hpp"
#include "CompilerProperties.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Settings::CompilerProperties::Open() {
	preprocessorDefinitions.clear();
	std::filesystem::path sceneListFile = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings/scenesManifest.txt";
	auto sceneListFilePath = sceneListFile.string();
	auto fileContents = Utils::LoadFileText(sceneListFilePath.c_str());

	size_t start = 0, end;
	std::string sceneName;
	while (true) {
		end = fileContents.find(";", start);
		if (end == std::string::npos) {
			sceneName = fileContents.substr(start);
			if (!sceneName.empty()) {
				preprocessorDefinitions.push_back(sceneName);
			}

			break;
		}

		sceneName = fileContents.substr(start, end - start);
		preprocessorDefinitions.push_back(sceneName);
		start = end + 1;
	}
}

void Settings::CompilerProperties::Render() {
	ImGui::Text("Compiler Properties");
	ImGui::Separator();
	ImGui::Text("Preprocessor Defines:");
	Components::ListEditor(preprocessorDefinitions, "PreprocessorDefines");

	if (ImGui::Button("Save")) {
		WriteFile();
	}
}

void Settings::CompilerProperties::WriteFile() {
	std::filesystem::path settingsFile = Editor::Manager::GetInstance().GetProjectPath() / "userSettings/compilerProperties.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	auto settingsPath = settingsFile.string();
	std::ofstream outputFile(settingsPath.c_str());

	if (!outputFile.is_open()) {
		return;
	}

	size_t numDefs = preprocessorDefinitions.size();

	std::string contents = "";
	for (auto i = 0; i < numDefs - 1; ++i) {
		contents += preprocessorDefinitions[i] + ";";
	}

	if (numDefs > 0) {
		contents += preprocessorDefinitions[numDefs - 1];
	}

	outputFile.clear();
	outputFile << contents;
	outputFile.close();
}
