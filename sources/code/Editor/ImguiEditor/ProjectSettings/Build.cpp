#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "Build.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/EditorManager.hpp"
#include "Editor/ImguiEditor/Components/ListEditor.hpp"
using namespace Grindstone::Editor::ImguiEditor::Settings;

void Build::Open() {
	sceneList.clear();
	std::filesystem::path sceneListFile = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings/scenesManifest.txt";
	auto sceneListFilePath = sceneListFile.string();
	auto fileContents = Utils::LoadFileText(sceneListFilePath.c_str());

	size_t start = 0, end;
	std::string sceneName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			sceneName = fileContents.substr(start);
			if (!sceneName.empty()) {
				sceneList.push_back(sceneName);
			}

			break;
		}

		sceneName = fileContents.substr(start, end - start);
		sceneList.push_back(sceneName);
		start = end + 1;
	}
}

void Build::Render() {
	ImGui::Text("Build Options");
	ImGui::Separator();

	ImGui::Text("Included Scenes:");
	Components::ListEditor(sceneList, "PrefScene");

	if (ImGui::Button("Save")) {
		WriteFile();
	}
}

void Build::WriteFile() {
	std::filesystem::path sceneListFile = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings/scenesManifest.txt";
	std::filesystem::create_directories(sceneListFile.parent_path());
	auto sceneListPath = sceneListFile.string();
	std::ofstream outputFile(sceneListPath.c_str());

	if (!outputFile.is_open()) {
		return;
	}

	std::string contents = "";
	if (sceneList.size() > 0) {
		for (auto i = 0; i < sceneList.size() - 1; ++i) {
			contents += sceneList[i] + "\n";
		}

		contents += sceneList[sceneList.size() - 1];
	}

	outputFile.clear();
	outputFile << contents;
	outputFile.close();
}
