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
			Manager(EngineCore* engine_core, ECS::Core* ecs_core);
			~Manager();
			
			bool load(const char* name);
			void loadCritical(const char* name);

			void remove(const char* name);
		private:
			Interface interface_;
			EngineCore *engine_core_;
			std::map<std::string, Utilities::Modules::Handle> plugins_;
		};
	}
}