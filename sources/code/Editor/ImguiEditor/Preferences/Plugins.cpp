#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/PluginSystem/Manager.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include "Editor/EditorManager.hpp"

#include "Plugins.hpp"
using namespace Grindstone::Editor::ImguiEditor;

void Preferences::Plugins::Open() {
	pluginList.clear();
	std::filesystem::path pluginListFile = Editor::Manager::GetInstance().GetProjectPath() / "buildSettings/pluginsManifest.txt";
	auto pluginListFilePath = pluginListFile.string();
	auto fileContents = Utils::LoadFileText(pluginListFilePath.c_str());

	size_t start = 0, end;
	std::string pluginName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			pluginName = fileContents.substr(start);
			if (!pluginName.empty()) {
				pluginList.push_back(pluginName);
			}

			break;
		}

		pluginName = fileContents.substr(start, end - start);
		pluginList.push_back(pluginName);
		start = end + 1;
	}
}

void Preferences::Plugins::Render() {
	ImGui::Text("Plugins Page");
	ImGui::Separator();

	if (ImGui::Button("+ Add Item")) {
		pluginList.emplace_back();
	}

	int itemToRemove = -1;
	for (auto i = 0; i < pluginList.size(); ++i) {
		std::string buttonLabel = std::string("-##PrefPluginRemBtn") + std::to_string(i);
		if (ImGui::Button(buttonLabel.c_str())) {
			itemToRemove = i;
		}

		ImGui::SameLine();
		std::string label = std::string("##PrefPluginField") + std::to_string(i);
		ImGui::InputText(label.c_str(), &pluginList[i]);
	}

	if (itemToRemove > -1) {
		pluginList.erase(pluginList.begin() + itemToRemove);
	}

	if (ImGui::Button("Save")) {
		hasPluginsChanged = true;
		WriteFile();
		// Editor::Manager::GetEngineCore().GetPluginManager()->LoadPluginList();
	}

	if (hasPluginsChanged) {
		ImGui::Text("Please restart the application for your changes to take effect.");
	}
}

void Preferences::Plugins::WriteFile() {
	std::string contents = "";
	for (auto i = 0; i < pluginList.size() - 1; ++i) {
		contents += pluginList[i] + "\n";
	}

	if (pluginList.size() > 0) {
		contents += pluginList[pluginList.size() - 1];
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
