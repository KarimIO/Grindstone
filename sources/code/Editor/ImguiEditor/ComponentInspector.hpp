#pragma once

#include <string>
#include <entt/entt.hpp>
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
#include "NewComponentInput.hpp"

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ComponentInspector {
			public:
				void Render(ECS::Entity entity);
			private:
				void RenderComponent(
					const char* componentTypeName,
					Reflection::TypeDescriptor_Struct& componentReflectionData,
					void* entity
				);
				void RenderComponentCategory(
					Reflection::TypeDescriptor_Struct::Category& category,
					void* entity
				);
				void RenderComponentMember(
					Reflection::TypeDescriptor_Struct::Member& member,
					void* entity
				);
				NewComponentInput newComponentInput;
			};
		}
	}
}
