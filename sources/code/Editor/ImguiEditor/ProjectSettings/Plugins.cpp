#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Editor/ImguiEditor/Components/ListEditor.hpp"

#include "Editor/EditorManager.hpp"

#include "Plugins.hpp"
using namespace Grindstone::Editor::ImguiEditor;

bool isPluginPopupShown = false;
size_t indexToEdit = SIZE_MAX;

const char* availablePlugins[] = {
	"PluginEditorAudioImporter",
	"PluginEditorMaterialImporter",
	"PluginEditorModelImporter",
	"PluginEditorPipelineSetImporter",
	"PluginEditorTextureImporter",
	"PluginBulletPhysics",
	"PluginScriptCSharp",
	"PluginRenderables3D",
	"PluginAudioOpenAL"
};

std::vector<std::string> unusedPlugins;

const ImVec2 PLUGIN_LIST_WINDOW_SIZE = { 300.f, 400.f };

static void FilterPlugins(std::vector<std::string>& usedPlugins) {
	unusedPlugins.clear();

	size_t availablePluginCount = sizeof(availablePlugins) / sizeof(availablePlugins[0]);
	for (size_t i = 0; i < availablePluginCount; ++i) {
		bool isPluginFound = false;

		for (size_t j = 0; j < usedPlugins.size(); ++j) {
			if (usedPlugins[j] == availablePlugins[i]) {
				isPluginFound = true;
				break;
			}
		}

		if (!isPluginFound) {
			unusedPlugins.emplace_back(availablePlugins[i]);
		}
	}
}

static void RenderPluginList(std::vector<std::string>& pluginList) {
	if (isPluginPopupShown) {
		ImGui::OpenPopup("Plugin List");
		isPluginPopupShown = false;
	}

	float width = 200.0f;

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowSize(PLUGIN_LIST_WINDOW_SIZE);

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 windowPos = ImVec2(
		(displaySize.x - PLUGIN_LIST_WINDOW_SIZE.x) / 2.0f,
		(displaySize.y - PLUGIN_LIST_WINDOW_SIZE.y) / 2.0f
	);
	ImGui::SetNextWindowPos(windowPos);

	if (ImGui::BeginPopupModal("Plugin List", false, flags)) {
		if (unusedPlugins.size() == 0) {
			ImGui::Text("No unused plugins found.");
			return;
		}

		const float width = ImGui::GetContentRegionAvail().x;

		for (size_t index = 0; index < unusedPlugins.size(); ++index) {
			if (ImGui::Button(unusedPlugins[index].c_str(), ImVec2(width, 0.0f))) {
				pluginList[indexToEdit] = unusedPlugins[index];

				ImGui::CloseCurrentPopup();
				isPluginPopupShown = false;
			}
		}

		ImGui::EndPopup();
	}
}

static void OnRenderPlugin(void* listPtr, size_t index, float width) {
	std::vector<std::string>& plugins = *static_cast<std::vector<std::string>*>(listPtr);

	const char* pluginText = !plugins[index].empty()
		? plugins[index].c_str()
		: "[ Unset ]";

	if (ImGui::Button(pluginText, ImVec2(width, 0.0f))) {
		isPluginPopupShown = true;
		indexToEdit = index;
		FilterPlugins(plugins);

		if (!plugins[index].empty()) {
			unusedPlugins.emplace_back(plugins[index]);
		}
	}
}

static void OnAddPlugin(void* listPtr, size_t index) {
	std::vector<std::string>& plugins = *static_cast<std::vector<std::string>*>(listPtr);

	plugins.emplace_back();
}

static void OnRemovePlugins(void* listPtr, size_t startIndex, size_t lastIndex) {
	std::vector<std::string>& plugins = *static_cast<std::vector<std::string>*>(listPtr);

	plugins.erase(plugins.begin() + startIndex, plugins.begin() + lastIndex + 1);
}

void Settings::Plugins::Open() {
	pluginList.clear();
	std::filesystem::path pluginListFile = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings/pluginsManifest.txt";
	auto pluginListFilePath = pluginListFile.string();
	auto fileContents = Utils::LoadFileText(pluginListFilePath.c_str());

	size_t start = 0, end;
	std::string pluginName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			pluginName = Utils::Trim(fileContents.substr(start));
			if (!pluginName.empty()) {
				pluginList.push_back(pluginName);
			}

			break;
		}

		pluginName = Utils::Trim(fileContents.substr(start, end - start));
		if (!pluginName.empty()) {
			pluginList.push_back(pluginName);
		}
		start = end + 1;
	}
}

void Settings::Plugins::Render() {
	ImGui::Text("Plugins Page");
	ImGui::Separator();

	const std::string listName = "Plugin List";
	Widgets::ListEditor(
		listName,
		&pluginList,
		pluginList.size(),
		OnRenderPlugin,
		OnAddPlugin,
		OnRemovePlugins
	);

	RenderPluginList(pluginList);

	if (ImGui::Button("Save")) {
		hasPluginsChanged = true;
		WriteFile();
		// Editor::Manager::GetEngineCore().GetPluginManager()->LoadPluginList();
	}

	if (hasPluginsChanged) {
		ImGui::Text("Please restart the application for your changes to take effect.");
	}
}

void Settings::Plugins::WriteFile() {
	std::string contents;
	for (auto i = 0; i < pluginList.size(); ++i) {
		if (!pluginList[i].empty()) {
			contents += pluginList[i] + "\n";
		}
	}

	std::filesystem::path pluginListFile = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings/pluginsManifest.txt";
	std::filesystem::create_directories(pluginListFile.parent_path());
	auto pluginListPath = pluginListFile.string();
	std::ofstream outputFile(pluginListPath.c_str());

	if (outputFile) {
		outputFile.clear();
		outputFile << contents;
		outputFile.close();
	}
}
