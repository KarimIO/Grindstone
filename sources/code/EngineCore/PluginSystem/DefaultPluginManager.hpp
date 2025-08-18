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
		virtual void LoadPluginsByStage(const char* stageName) override;
		virtual void UnloadPluginsByStage(const char* stageName) override;

	protected:
		bool LoadModule(const std::string& path);
		void UnloadModule(const std::string& path);

		std::map<std::string, Utilities::Modules::Handle> pluginModules;
	};
}
