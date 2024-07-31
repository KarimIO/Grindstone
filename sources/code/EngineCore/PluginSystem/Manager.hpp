#pragma once

#include <map>
#include <string>

#include <Common/Utilities/ModuleLoading.hpp>

#include "Interface.hpp"

namespace Grindstone {
	class EngineCore;

	namespace Plugins {
		class Interface;

		class Manager {
			friend Interface;
		public:
			Manager(Grindstone::EngineCore* engineCore);
			~Manager();

			void SetupInterfacePointers();
			virtual Interface& GetInterface();
			
			virtual void LoadPluginList();
			virtual void UnloadPluginList();
			bool Load(const char* name);
			void LoadCritical(const char* name);

			void Remove(const char* name);
		private:
			Interface pluginInterface;
			EngineCore *engineCore;
			std::map<std::string, Utilities::Modules::Handle> plugins;
			std::vector<Utilities::Modules::Handle> pluginsFromList;
		};
	}
}
