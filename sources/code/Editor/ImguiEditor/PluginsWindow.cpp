#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/ImguiEditor/Components/ListEditor.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/PluginSystem/PluginMetaFileLoader.hpp>

#include "imgui_markdown.h"
#include "PluginsWindow.hpp"
using namespace Grindstone::Editor::ImguiEditor;

bool isPluginPopupShown = false;
size_t indexToEdit = SIZE_MAX;

std::vector<std::string> unusedPlugins;

const ImVec2 PLUGIN_LIST_WINDOW_SIZE = { 300.f, 400.f };

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

	const ImguiEditor& imguiEditor = Grindstone::Editor::Manager::GetInstance().GetImguiEditor();

	size_t newSelectedIndex = SIZE_MAX;
	size_t currentIndex = 0;
	ImVec2 padding(8, 8);
	for (const auto& plugin : pluginsList) {
		bool isSelected = (currentIndex == currentSelectedPlugin);

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(p0.x + padding.x, p0.y + padding.y));

		ImGui::BeginGroup();
		ImGui::PushFont(imguiEditor.GetFont(FontType::H3));
		ImGui::Text(plugin.displayName.c_str());
		ImGui::PopFont();

		ImGui::PushFont(imguiEditor.GetFont(FontType::Italic));
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
		ImGui::Text(plugin.author.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::TextWrapped(plugin.description.c_str());
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
	const ImguiEditor& imguiEditor = Grindstone::Editor::Manager::GetInstance().GetImguiEditor();

	ImGui::PushFont(imguiEditor.GetFont(FontType::H1));
	ImGui::Text(pluginManifestCache.displayName.c_str());
	ImGui::PopFont();

	ImGui::PushFont(imguiEditor.GetFont(FontType::Italic));
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
	ImGui::Text(pluginManifestCache.author.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();

	ImGui::TextWrapped(pluginManifestCache.description.c_str());


	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("PluginPageTabs", tab_bar_flags)) {
		if (ImGui::BeginTabItem("Readme")) {
			if (currentPluginData.readmeData.empty()) {
				ImGui::Text("No readme found.");
			}
			else {
				const ImGui::MarkdownConfig& mdConfig = imguiEditor.GetMarkdownConfig();
				ImGui::Markdown(currentPluginData.readmeData.c_str(), currentPluginData.readmeData.size(), mdConfig);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Dependencies")) {
			ImGui::Text("Dependencies tab not implemented yet.");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Content")) {
			ImGui::Text("Content tab not implemented yet.");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
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
	std::vector<std::string> usedPlugins;

	{
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
					usedPlugins.emplace_back(pluginName);
				}

				break;
			}

			pluginName = Utils::Trim(fileContents.substr(start, end - start));
			if (!pluginName.empty()) {
				usedPlugins.emplace_back(pluginName);
			}
			start = end + 1;
		}
	}

	std::filesystem::path pluginsPath = Editor::Manager::GetInstance().GetEngineBinariesPath().parent_path() / "plugins";
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(pluginsPath)) {
		if (!entry.is_directory()) {
			continue;
		}

		const std::filesystem::path& pluginFolderPath = entry.path();
		std::filesystem::path metaFilePath = pluginFolderPath / "plugin.meta.json";
		Grindstone::Plugins::MetaData metaData;
		if (!ReadMetaFile(metaFilePath, metaData)) {
			continue;
		}

		std::string pluginName = pluginFolderPath.filename().string();
		pluginCacheList.emplace_back(pluginName, metaData.displayName, metaData.description, metaData.author);
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
