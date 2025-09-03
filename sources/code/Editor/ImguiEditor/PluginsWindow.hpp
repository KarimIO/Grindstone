#pragma once

#include <vector>
#include <string>
#include "Settings/BaseSettingsPage.hpp"

namespace Grindstone::Editor::ImguiEditor {
	struct PluginManifestCache {
		std::string name;
		std::string displayName;
		std::string description;
		std::string author;
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
		void LoadPluginsManifest();
		void WriteFile();
		bool isOpen = false;
		std::vector<PluginManifestCache> pluginCacheList;
		CurrentPluginData currentPluginData;
		PluginSelectionState pluginSelectionState;
		size_t currentSelectedPlugin;
		bool hasPluginsChanged = false;
	};
}
