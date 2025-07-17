#pragma once

#include <string>
#include <entt/entt.hpp>

#include <Common/HashedString.hpp>
#include <EngineCore/Reflection/TypeDescriptorStruct.hpp>
#include "NewComponentInput.hpp"

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiEditor;
			class ComponentInspector {
			public:
				ComponentInspector(ImguiEditor* editor);
				void Render(ECS::Entity entity);
			private:
				void RenderComponent(
					Grindstone::HashedString componentTypeName,
					Reflection::TypeDescriptor_Struct& componentReflectionData,
					void* componentPtr,
					ECS::Entity entity
				);
				void RenderCSharpScript(void* componentPtr, ECS::Entity entity);
				void RenderComponentCategory(
					Reflection::TypeDescriptor_Struct::Category& category,
					void* componentPtr,
					ECS::Entity entity
				);
				void RenderComponentMember(
					Reflection::TypeDescriptor_Struct::Member& member,
					void* componentPtr,
					ECS::Entity entity
				);
				void RenderComponentMember(std::string_view displayName, Reflection::TypeDescriptor* itemType, void* offset, ECS::Entity entity);
				NewComponentInput newComponentInput;

				ImguiEditor* imguiEditor = nullptr;
			};
		}
	}
}
