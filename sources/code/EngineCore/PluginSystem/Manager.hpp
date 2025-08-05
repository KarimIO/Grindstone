#pragma once

#include <map>
#include <string>

#include <Common/Utilities/ModuleLoading.hpp>

#include "PluginMetaData.hpp"
#include "Interface.hpp"

namespace Grindstone {
	class EngineCore;
}

namespace Grindstone::Plugins {
	class Interface;

	class Manager {
		friend Interface;
	public:
		Manager(Grindstone::EngineCore* engineCore);
		~Manager();

		void SetupInterfacePointers();
		virtual Interface& GetInterface();
			
		virtual bool LoadPluginList();
		virtual void LoadPluginsOfStage(const char* stageName);
		virtual void UnloadPluginListExceptRenderHardwareInterface();
		virtual void UnloadPluginRenderHardwareInterface();
		bool Load(const std::filesystem::path& path);
		void LoadCritical(const std::filesystem::path& path);

		void Remove(const std::filesystem::path& path);
	private:
		Interface pluginInterface;
		EngineCore *engineCore;
		std::map<std::filesystem::path, Utilities::Modules::Handle> plugins;
		std::vector<Utilities::Modules::Handle> pluginsFromList;
		std::vector<Grindstone::Plugins::MetaData> resolvedPluginManifest{};
	};
}
