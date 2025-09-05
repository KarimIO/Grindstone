#include <fstream>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/ImguiEditor/Components/ListEditor.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/PluginSystem/EditorPluginManager.hpp>
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

static size_t OnRenderPluginSidebar(const std::vector<PluginListElement>& pluginsList, size_t currentSelectedPlugin) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));

	if (!ImGui::BeginChild("#SidebarPluginArea", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None)) {
		return SIZE_MAX;
	}

	const ImguiEditor& imguiEditor = Grindstone::Editor::Manager::GetInstance().GetImguiEditor();

	size_t newSelectedIndex = SIZE_MAX;
	size_t currentIndex = 0;
	ImVec2 padding(8, 8);
	for (const auto& plugin : pluginsList) {
		bool isSelected = (currentIndex == currentSelectedPlugin);

		std::string rowName = "#PluginRow" + plugin.metaData.displayName;

		if (isSelected) {
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}

		bool isInstalled = plugin.installationState == PluginInstallationState::Installed || plugin.installationState == PluginInstallationState::Installing;
		if (isInstalled) {
			ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
		}

		ImGui::BeginChild(rowName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0.0f), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AutoResizeY);

		ImGui::PushFont(imguiEditor.GetFont(FontType::H3));
		ImGui::Text(plugin.metaData.displayName.c_str());
		ImGui::PopFont();

		ImGui::PushFont(imguiEditor.GetFont(FontType::Italic));
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		ImGui::Text(plugin.metaData.author.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::TextWrapped(plugin.metaData.description.c_str());
		ImGui::EndChild();

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			newSelectedIndex = currentIndex;
		}

		if (isInstalled) {
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}

		if (isSelected) {
			ImGui::PopStyleColor();
		}

		++currentIndex;
	}

	ImGui::EndChild();

	ImGui::PopStyleVar(2);

	return newSelectedIndex;
}

static void OnRenderPluginPageSuccess(PluginListElement& pluginManifestCache, const CurrentPluginData& currentPluginData) {
	const ImguiEditor& imguiEditor = Grindstone::Editor::Manager::GetInstance().GetImguiEditor();

	ImGui::PushFont(imguiEditor.GetFont(FontType::H1));
	ImGui::Text(pluginManifestCache.metaData.displayName.c_str());
	ImGui::PopFont();

	ImGui::PushFont(imguiEditor.GetFont(FontType::Italic));
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
	ImGui::Text(pluginManifestCache.metaData.author.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();

	ImGui::TextWrapped(pluginManifestCache.metaData.description.c_str());
	switch (pluginManifestCache.installationState) {
	case PluginInstallationState::NotInstalled:
		if (ImGui::Button("Install")) {
			Grindstone::Plugins::EditorPluginManager* pluginManager = static_cast<Grindstone::Plugins::EditorPluginManager*>(Grindstone::EngineCore::GetInstance().GetPluginManager());
			pluginManager->QueueInstall(pluginManifestCache.metaData.name);
			pluginManifestCache.installationState = PluginInstallationState::Installed;
		}
		break;
	case PluginInstallationState::Installed:
		if (ImGui::Button("Uninstall")) {
			Grindstone::Plugins::EditorPluginManager* pluginManager = static_cast<Grindstone::Plugins::EditorPluginManager*>(Grindstone::EngineCore::GetInstance().GetPluginManager());
			pluginManager->QueueUninstall(pluginManifestCache.metaData.name);
			pluginManifestCache.installationState = PluginInstallationState::NotInstalled;
		}
		break;
	case PluginInstallationState::Uninstalling:
		ImGui::BeginDisabled(true);
		ImGui::Button("Uninstalling...");
		ImGui::EndDisabled();
		break;
	case PluginInstallationState::Installing:
		ImGui::BeginDisabled(true);
		ImGui::Button("Installing...");
		ImGui::EndDisabled();
		break;
	}
	ImGui::NewLine();

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

static void OnRenderPluginPage(std::vector<PluginListElement>& pluginsList, size_t currentSelectedPlugin, const CurrentPluginData& currentPluginData, const PluginSelectionState currentSelectionState) {
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

void PluginsWindow::SelectPlugin(size_t newSelectedIndex) {
	currentSelectedPlugin = newSelectedIndex;
	pluginSelectionState = PluginSelectionState::Ready;
	const std::string& pluginName = pluginCacheList[newSelectedIndex].metaData.name;
	std::filesystem::path pathToReadme = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins" / pluginName / "README.md";
	currentPluginData.readmeData = Utils::LoadFileText(pathToReadme.string().c_str());
}

void PluginsWindow::LoadPluginsManifest() {
	pluginCacheList.clear();

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

		pluginCacheList.emplace_back(metaData, Grindstone::Editor::ImguiEditor::PluginInstallationState::NotInstalled);
	}

	std::vector<std::string> usedPlugins;
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

	for (const std::string& usedPluginEntry : usedPlugins) {
		for (Grindstone::Editor::ImguiEditor::PluginListElement& listElement : pluginCacheList) {
			if (listElement.metaData.name == usedPluginEntry) {
				listElement.installationState = Grindstone::Editor::ImguiEditor::PluginInstallationState::Installed;
			}
		}
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
				SelectPlugin(newSelectedIndex);
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
		if (!pluginCacheList[i].metaData.name.empty()) {
			contents += pluginCacheList[i].metaData.name + "\n";
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
