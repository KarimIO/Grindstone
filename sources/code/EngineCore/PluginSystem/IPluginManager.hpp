#pragma once

#include <string_view>
#include <filesystem>

namespace Grindstone::Plugins {
	class Interface;

	class IPluginManager {
	public:
		virtual ~IPluginManager() {}

		virtual bool PreprocessPlugins() = 0;
		virtual void LoadPluginsByStage(std::string_view stageName) = 0;
		virtual void UnloadPluginsByStage(std::string_view stageName) = 0;
		virtual std::filesystem::path GetLibraryPath(std::string_view pluginName, std::string_view libraryName) = 0;
	};
}
