#pragma once

#include <map>
#include <string>

#include <Common/Utilities/ModuleLoading.hpp>

#include "IPluginManager.hpp"
#include "Interface.hpp"

namespace Grindstone::Plugins {
	class Interface;

	class DefaultPluginManager : public Grindstone::Plugins::IPluginManager {
	public:
		virtual ~DefaultPluginManager();
	
		virtual bool PreprocessPlugins() override;
		virtual void LoadPluginsByStage(std::string_view stageName) override;
		virtual void UnloadPluginsByStage(std::string_view stageName) override;
		virtual std::filesystem::path GetLibraryPath(std::string_view pluginName, std::string_view libraryName) override;

	protected:
		bool LoadModule(const std::string& path);
		void UnloadModule(const std::string& path);

		std::map<std::string, Utilities::Modules::Handle> pluginModules;
	};
}
