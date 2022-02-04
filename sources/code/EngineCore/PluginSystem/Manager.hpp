#pragma once

#include <string>
#include <map>

#include <Common/Utilities/ModuleLoading.hpp>
#include "Interface.hpp"

namespace Grindstone {
	class EngineCore;

	namespace Plugins {
		class Manager {
			friend class Interface;
		public:
			Manager(EngineCore* engineCore);
			~Manager();

			void SetupInterfacePointers();
			
			virtual void LoadPluginList();
			bool Load(const char* name);
			void LoadCritical(const char* name);

			void Remove(const char* name);
		private:
			Interface pluginInterface;
			EngineCore *engineCore;
			std::map<std::string, Utilities::Modules::Handle> plugins;
		};
	}
}
