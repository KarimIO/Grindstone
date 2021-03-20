#pragma once

#include <entt/entt.hpp>
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
		class SceneManager;
	}

	namespace ECS {
		class ComponentRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ComponentInspector {
			public:
				void render(ECS::ComponentRegistrar& registrar, entt::registry& registry, entt::entity entity);
			private:
				void renderComponent(
					const char* componentTypeName,
					Reflection::TypeDescriptor_Struct& componentReflectionData,
					void* entity
				);
				void renderComponentCategory(
					Reflection::TypeDescriptor_Struct::Category& category,
					void* entity
				);
				void renderComponentMember(
					Reflection::TypeDescriptor_Struct::Member& member,
					void* entity
				);
				bool isShowingPanel = true;
			};
		}
	}
}
