#pragma once

#include <string>
#include <entt/entt.hpp>
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
#include "NewComponentInput.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace ECS {
		class ComponentRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ComponentInspector {
			public:
				void render(ECS::ComponentRegistrar& registrar, SceneManagement::Scene* scene, entt::entity entity);
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
				NewComponentInput newComponentInput;
			};
		}
	}
}
