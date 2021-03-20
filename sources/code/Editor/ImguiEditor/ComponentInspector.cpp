#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/Reflection/TypeDescriptor.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void ComponentInspector::render(
				ECS::ComponentRegistrar& registrar,
				entt::registry& registry,
				entt::entity entity
			) {
				for each (auto componentEntry in registrar) {
					const char* componentTypeName = componentEntry.first.c_str();
					auto componentReflectionData = componentEntry.second.getComponentReflectionDataFn();
					auto tryGetComponentFn = componentEntry.second.tryGetComponentFn;

					void* outEntity = nullptr;
					if (tryGetComponentFn(registry, entity, outEntity)) {
						renderComponent(componentTypeName, componentReflectionData, outEntity);
					}
				}
			}

			void ComponentInspector::renderComponent(
				const char* componentTypeName,
				Grindstone::Reflection::TypeDescriptor_Struct& componentReflectionData,
				void* entity
			) {
				if (!ImGui::TreeNode(componentTypeName)) {
					return;
				}

				ImGui::Text(componentTypeName);
				renderComponentCategory(componentReflectionData.category, entity);
				ImGui::Separator();

				ImGui::TreePop();
			}
			
			void ComponentInspector::renderComponentCategory(
				Reflection::TypeDescriptor_Struct::Category& category,
				void* entity
			) {
				for each (auto subcategory in category.categories) {
					if (!ImGui::TreeNode(subcategory.name.c_str())) {
						return;
					}

					renderComponentCategory(subcategory, entity);

					ImGui::TreePop();
				}

				for each (auto member in category.members) {
					renderComponentMember(member, entity);
				}
			}

			void ComponentInspector::renderComponentMember(
				Reflection::TypeDescriptor_Struct::Member& member,
				void* entity
			) {
				char* offset = ((char*)entity + member.offset);
				switch(member.type->type) {
				case Reflection::TypeDescriptor::ReflectionTypeData::ReflString:
					ImGui::InputText(
						member.display_name.c_str(),
						(std::string *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::ReflVec3:
					ImGui::InputFloat3(
						member.display_name.c_str(),
						(float *)offset
					);
					break;
				}
			}
		}
	}
}
