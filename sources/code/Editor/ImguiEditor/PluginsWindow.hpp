#pragma once

#include <vector>
#include <string>
#include "Settings/BaseSettingsPage.hpp"
#include <Editor/PluginSystem/PluginMetaData.hpp>

namespace Grindstone::Editor::ImguiEditor {
	enum class PluginInstallationState {
		NotInstalled,
		Installed,
		Uninstalling,
		Installing,
	};

	struct PluginListElement {
		Grindstone::Plugins::MetaData metaData;
		PluginInstallationState installationState;
	};

	struct CurrentPluginData {
		std::string readmeData;
	};

	enum class PluginSelectionState {
		NotSelected,
		Loading,
		Ready
	};

	class PluginsWindow {
	public:
		void Open();
		void Render();
		bool IsOpen() const;
	private:
		void SelectPlugin(size_t newSelectedIndex);
		void LoadPluginsManifest();
		void WriteFile();
		bool isOpen = false;
		std::vector<PluginListElement> pluginCacheList;
		CurrentPluginData currentPluginData;
		PluginSelectionState pluginSelectionState;
		size_t currentSelectedPlugin;
		bool hasPluginsChanged = false;
	};
}
