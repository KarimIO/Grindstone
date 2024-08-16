#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/ImguiEditor/Components/ListEditor.hpp>

#include "Build.hpp"

using namespace Grindstone::Editor::ImguiEditor::Settings;

size_t settingSceneIndex = SIZE_MAX;

static void OnRenderScene(void* listPtr, size_t index, float width) {
	std::vector<Build::SceneData>& scenes = *static_cast<std::vector<Build::SceneData>*>(listPtr);

	Build::SceneData& sceneValue = scenes[index];

	const char* scenesText = !sceneValue.displayName.empty()
		? sceneValue.displayName.c_str()
		: "[ Unset ]";

	if (ImGui::Button(scenesText, ImVec2(width, 0.0f))) {
		Grindstone::Editor::Manager::GetInstance().GetImguiEditor().PromptAssetPicker(
			Grindstone::AssetType::Scene,
			[&sceneValue](Grindstone::Uuid assetUuid, std::string assetName) {
				sceneValue.uuid = assetUuid;
				sceneValue.displayName = assetName;
			}
		);
	}
}

static void OnAddScene(void* listPtr, size_t index) {
	std::vector<Build::SceneData>& scenes = *static_cast<std::vector<Build::SceneData>*>(listPtr);

	scenes.emplace_back();
}

static void OnRemoveScenes(void* listPtr, size_t startIndex, size_t lastIndex) {
	std::vector<Build::SceneData>& scenes = *static_cast<std::vector<Build::SceneData>*>(listPtr);

	scenes.erase(scenes.begin() + startIndex, scenes.begin() + lastIndex + 1);
}

void Build::Open() {
	Editor::Manager& editor = Editor::Manager::GetInstance();
	Editor::AssetRegistry& assetRegistry = editor.GetAssetRegistry();

	sceneList.clear();
	std::filesystem::path sceneListFile = editor.GetProjectPath() / "buildSettings/scenesManifest.txt";
	auto sceneListFilePath = sceneListFile.string();
	auto fileContents = Utils::LoadFileText(sceneListFilePath.c_str());

	size_t start = 0, end;
	std::string sceneUuidAsString;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			sceneUuidAsString = Utils::Trim(fileContents.substr(start));
			if (!sceneUuidAsString.empty()) {
				Uuid uuid = Uuid(sceneUuidAsString);
				AssetRegistry::Entry entry;
				if (assetRegistry.TryGetAssetData(uuid, entry)) {
					sceneList.emplace_back(uuid, entry.name);
				}
			}

			break;
		}

		sceneUuidAsString = Utils::Trim(fileContents.substr(start, end - start));
		{
			Uuid uuid = Uuid(sceneUuidAsString);
			AssetRegistry::Entry entry;
			if (assetRegistry.TryGetAssetData(uuid, entry)) {
				sceneList.emplace_back(uuid, entry.name);
			}
		}
		start = end + 1;
	}
}

void Build::Render() {
	ImGui::Text("Build Options");
	ImGui::Separator();

	const std::string listName = "Plugin List";
	Widgets::ListEditor(
		listName,
		&sceneList,
		sceneList.size(),
		OnRenderScene,
		OnAddScene,
		OnRemoveScenes
	);

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
	for (auto i = 0; i < sceneList.size(); ++i) {
		contents += sceneList[i].uuid.ToString() + "\n";
	}

	outputFile.clear();
	outputFile << contents;
	outputFile.close();
}
