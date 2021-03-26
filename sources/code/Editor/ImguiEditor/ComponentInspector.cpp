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

				renderComponentCategory(componentReflectionData.category, entity);

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
				const char* displayName = member.displayName.c_str();
				char* offset = ((char*)entity + member.offset);
				switch(member.type->type) {
				case Reflection::TypeDescriptor::ReflectionTypeData::Bool:
					ImGui::Checkbox(
						displayName,
						(bool*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::String:
					ImGui::InputText(
						displayName,
						(std::string *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int:
					ImGui::InputInt(
						displayName,
						(int*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int2:
					ImGui::InputInt2(
						displayName,
						(int *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int3:
					ImGui::InputInt3(
						displayName,
						(int *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int4:
					ImGui::InputInt4(
						displayName,
						(int *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float:
					ImGui::InputFloat(
						displayName,
						(float*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float2:
					ImGui::InputFloat2(
						displayName,
						(float *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float3:
					ImGui::InputFloat3(
						displayName,
						(float *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float4:
					ImGui::InputFloat4(
						displayName,
						(float *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Double:
					ImGui::InputDouble(
						displayName,
						(double*)offset
					);
					break;
				}
			}
		}
	}
}
