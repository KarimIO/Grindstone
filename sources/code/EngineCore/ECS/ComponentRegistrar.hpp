#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <string>

#include "EngineCore/ECS/Entity.hpp"
#include "ComponentFactory.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar {
		public:
			ComponentRegistrar();
			void registerComponent(const char *name, ComponentFactory factory);
			void* createComponent(const char *name, entt::registry& registry, ECS::Entity entity);
			~ComponentRegistrar();
		private:
			std::unordered_map<std::string, ComponentFactory> componentFactories;
		};
	}
}
