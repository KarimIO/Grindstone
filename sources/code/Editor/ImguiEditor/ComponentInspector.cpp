#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "Editor/EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/Reflection/TypeDescriptor.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void ComponentInspector::Render(ECS::Entity entity) {
				ImGui::Text("Entity ID: %u", (uint32_t)entity.GetHandle());

				auto& editorManager = Editor::Manager::GetInstance();
				ECS::ComponentRegistrar& componentRegistrar = *editorManager.GetEngineCore().GetComponentRegistrar();
				std::vector<std::string> unusedComponentsItems;
				std::vector<ECS::ComponentFunctions> unusedComponentsFunctions;
				for each (auto componentEntry in componentRegistrar) {
					const char* componentTypeName = componentEntry.first.c_str();
					auto componentReflectionData = componentEntry.second.GetComponentReflectionDataFn();
					auto tryGetComponentFn = componentEntry.second.TryGetComponentFn;

					void* outComponent = nullptr;
					if (entity.TryGetComponent(componentTypeName, outComponent)) {
						RenderComponent(componentTypeName, componentReflectionData, outComponent);
					}
					else {
						unusedComponentsItems.push_back(componentEntry.first);
						unusedComponentsFunctions.push_back(componentEntry.second);
					}
				}

				ImGui::Separator();

				ImGui::Text("Attach a component:");
				newComponentInput.Render(
					entity,
					unusedComponentsItems
				);
			}

			void ComponentInspector::RenderComponent(
				const char* componentTypeName,
				Grindstone::Reflection::TypeDescriptor_Struct& componentReflectionData,
				void* entity
			) {
				if (!ImGui::TreeNode(componentTypeName)) {
					return;
				}

				RenderComponentCategory(componentReflectionData.category, entity);

				ImGui::TreePop();
			}
			
			void ComponentInspector::RenderComponentCategory(
				Reflection::TypeDescriptor_Struct::Category& category,
				void* entity
			) {
				for each (auto subcategory in category.categories) {
					if (!ImGui::TreeNode(subcategory.name.c_str())) {
						return;
					}

					RenderComponentCategory(subcategory, entity);

					ImGui::TreePop();
				}

				for each (auto member in category.members) {
					RenderComponentMember(member, entity);
				}
			}

			void ComponentInspector::RenderComponentMember(
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
						(float*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion:
					ImGui::InputFloat4(
						displayName,
						(float*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Double:
					ImGui::InputDouble(
						displayName,
						(double*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Vector:
					std::vector<std::string>& vector = *(std::vector<std::string>*)offset;
					ImGui::Text(displayName);
					ImGui::SameLine();
					std::string buttonFieldName = std::string("+##") + displayName;
					if (ImGui::Button(buttonFieldName.c_str())) {
						vector.emplace_back();
					}

					for (size_t i = 0; i < vector.size(); ++i) {
						std::string fieldName = std::string("##") + std::to_string(i) + displayName;
						ImGui::InputText(fieldName.c_str(), &vector[i]);
						ImGui::SameLine();
						std::string eraseFieldName = std::string("-") + fieldName;
						if (ImGui::Button(eraseFieldName.c_str())) {
							vector.erase(vector.begin() + i);
						}
					}
					break;
				}
			}
		}
	}
}
