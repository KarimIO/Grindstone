#pragma once

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

#include "SystemFactory.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		class SystemRegistrar {
		public:
			SystemRegistrar();
			void RegisterSystem(const char *name, SystemFactory factory);
			void Update(entt::registry& registry);
			~SystemRegistrar();
			std::unordered_map<std::string, SystemFactory> systemFactories;
		private:
		};
	}
}
