#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/ImguiEditor/Components/ListEditor.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>

#include "imgui_markdown.h"
#include "PluginsWindow.hpp"
using namespace Grindstone::Editor::ImguiEditor;

bool isPluginPopupShown = false;
size_t indexToEdit = SIZE_MAX;

const char* availablePlugins[] = {
	"Grindstone.RHI.DirectX12",
	"Grindstone.RHI.OpenGL",
	"Grindstone.RHI.Vulkan",
	"Grindstone.Editor.AudioImporter",
	"Grindstone.Editor.MaterialImporter",
	"Grindstone.Editor.ModelImporter",
	"Grindstone.Editor.PipelineSetImporter",
	"Grindstone.Editor.TextureImporter",
	"Grindstone.Audio.OpenAL",
	"Grindstone.Physics.Bullet",
	"Grindstone.Renderables.3D",
	"Grindstone.Renderer.Deferred",
	"Grindstone.Script.CSharp"
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

	bool isOpen = false;
	if (ImGui::BeginPopupModal("Plugin List", &isOpen, flags)) {
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

static size_t OnRenderPluginSidebar(const std::vector<PluginManifestCache>& pluginsList, size_t currentSelectedPlugin) {
	if (!ImGui::BeginChild("#SidebarPluginArea", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None)) {
		return SIZE_MAX;
	}

	size_t newSelectedIndex = SIZE_MAX;
	size_t currentIndex = 0;
	ImVec2 padding(8, 8);
	for (const auto& plugin : pluginsList) {
		bool isSelected = (currentIndex == currentSelectedPlugin);

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(p0.x + padding.x, p0.y + padding.y));

		ImGui::BeginGroup();
		ImGui::Text(plugin.displayName.c_str());
		ImGui::Text(plugin.description.c_str());
		ImGui::Text(plugin.author.c_str());
		ImGui::EndGroup();

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			newSelectedIndex = currentIndex;
		}

		ImVec2 p1 = ImGui::GetItemRectMax();
		p1 = ImVec2(p1.x + padding.x, p1.y + padding.y);

		ImGui::GetWindowDrawList()->AddRect(p0, p1, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_TitleBg)));

		++currentIndex;
	}

	ImGui::EndChild();

	return newSelectedIndex;
}

static void OnRenderPluginPageSuccess(const PluginManifestCache& pluginManifestCache, const CurrentPluginData& currentPluginData) {
	ImGui::Text(pluginManifestCache.displayName.c_str());
	ImGui::Text(pluginManifestCache.description.c_str());
	ImGui::Text(pluginManifestCache.author.c_str());

	const ImGui::MarkdownConfig& mdConfig = Grindstone::Editor::Manager::GetInstance().GetImguiEditor().GetMarkdownConfig();
	ImGui::Markdown(currentPluginData.readmeData.c_str(), currentPluginData.readmeData.size(), mdConfig);
}

static void OnRenderPluginPage(const std::vector<PluginManifestCache>& pluginsList, size_t currentSelectedPlugin, const CurrentPluginData& currentPluginData, const PluginSelectionState currentSelectionState) {
	if (!ImGui::BeginChild("#MainPluginArea", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None)) {
		return;
	}

	if (currentSelectedPlugin >= pluginsList.size()) {
		ImGui::Text("No plugin selected.");
	}
	else {
		switch (currentSelectionState) {
		case PluginSelectionState::NotSelected:
			ImGui::Text("No plugin selected.");
			break;
		case PluginSelectionState::Loading:
			ImGui::Text("Plugin data loading...");
			break;
		case PluginSelectionState::Ready:
			OnRenderPluginPageSuccess(pluginsList[currentSelectedPlugin], currentPluginData);
			break;
		}
	}

	ImGui::EndChild();
}

void PluginsWindow::Open() {
	currentSelectedPlugin = SIZE_MAX;
	pluginSelectionState = PluginSelectionState::NotSelected;
	isOpen = true;
	LoadPluginsManifest();
}

void PluginsWindow::LoadPluginsManifest() {
	pluginCacheList.clear();
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
				pluginCacheList.emplace_back(pluginName, pluginName + "Display Name", "Description", "Author");
			}

			break;
		}

		pluginName = Utils::Trim(fileContents.substr(start, end - start));
		if (!pluginName.empty()) {
			pluginCacheList.emplace_back(pluginName, pluginName + "Display Name", "Description", "Author");
		}
		start = end + 1;
	}
}

void PluginsWindow::Render() {
	if (!isOpen) {
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

	if (ImGui::Begin("Plugins", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar)) {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

		if (ImGui::BeginTable("SettingsSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX)) {
			ImGui::TableNextColumn();

			size_t newSelectedIndex = OnRenderPluginSidebar(pluginCacheList, currentSelectedPlugin);
			if (newSelectedIndex != SIZE_MAX) {
				currentSelectedPlugin = newSelectedIndex;
				pluginSelectionState = PluginSelectionState::Ready;
				const std::string& pluginName = pluginCacheList[newSelectedIndex].name;
				std::filesystem::path pathToReadme = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins" / pluginName / "README.md";
				currentPluginData.readmeData = Utils::LoadFileText(pathToReadme.string().c_str());
			}

			ImGui::TableNextColumn();

			OnRenderPluginPage(pluginCacheList, currentSelectedPlugin, currentPluginData, pluginSelectionState);
			
			ImGui::EndTable();
		}

		ImGui::PopStyleVar();
		ImGui::End();
	}

	ImGui::PopStyleVar();
}

void PluginsWindow::WriteFile() {
	std::string contents;
	for (auto i = 0; i < pluginCacheList.size(); ++i) {
		if (!pluginCacheList[i].name.empty()) {
			contents += pluginCacheList[i].name + "\n";
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

bool PluginsWindow::IsOpen() const {
	return isOpen;
}
