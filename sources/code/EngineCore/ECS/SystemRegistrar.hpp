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
			void RegisterSystem(const char* name, SystemFactory factory);
			void RegisterEditorSystem(const char* name, SystemFactory factory);
			void Update(entt::registry& registry);
			void EditorUpdate(entt::registry& registry);
			~SystemRegistrar();
			std::unordered_map<std::string, SystemFactory> systemFactories;
			std::unordered_map<std::string, SystemFactory> editorSystemFactories;
		private:
		};
	}
}
