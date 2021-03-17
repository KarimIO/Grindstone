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
			void registerSystem(const char *name, SystemFactory factory);
			void update(entt::registry& registry);
			~SystemRegistrar();
		private:
			std::unordered_map<std::string, SystemFactory> systemFactories;
		};
	}
}
