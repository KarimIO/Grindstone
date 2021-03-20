#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void ComponentInspector::render(
				ECS::ComponentRegistrar& registrar,
				entt::registry& registry,
				entt::entity entity
			) {
				for each (auto componentEntry in registrar) {
					auto tryGetComponentFn = componentEntry.second.tryGetComponentFn;
					
					void* outEntity = nullptr;
					if (tryGetComponentFn(registry, entity, outEntity)) {
						ImGui::Text(componentEntry.first.c_str());
					}
				}
			}
		}
	}
}
