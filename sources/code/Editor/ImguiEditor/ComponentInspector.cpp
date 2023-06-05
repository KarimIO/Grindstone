#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include <glm/gtc/quaternion.hpp>
#include "ComponentInspector.hpp"
#include "Editor/EditorManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/Reflection/TypeDescriptor.hpp"
#include "EngineCore/Assets/Asset.hpp"
// #include "EngineCore/Assets/Mesh3d/Mesh3"
// #include "EngineCore/Assets/Mesh3d/Mesh3d.hpp"
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
						RenderComponent(componentTypeName, componentReflectionData, outComponent, entity);
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
				void* componentPtr,
				ECS::Entity entity
			) {
				if (!ImGui::TreeNode(componentTypeName)) {
					return;
				}

				RenderComponentCategory(componentReflectionData.category, componentPtr, entity);

				ImGui::TreePop();
			}
			
			void ComponentInspector::RenderComponentCategory(
				Reflection::TypeDescriptor_Struct::Category& category,
				void* componentPtr,
				ECS::Entity entity
			) {
				for each (auto subcategory in category.categories) {
					if (!ImGui::TreeNode(subcategory.name.c_str())) {
						return;
					}

					RenderComponentCategory(subcategory, componentPtr, entity);

					ImGui::TreePop();
				}

				for each (auto member in category.members) {
					RenderComponentMember(member, componentPtr, entity);
				}
			}

			void ComponentInspector::RenderComponentMember(
				Reflection::TypeDescriptor_Struct::Member& member,
				void* componentPtr,
				ECS::Entity entity
			) {
				void* offset = ((char*)componentPtr + member.offset);
				RenderComponentMember(std::string_view(member.displayName), member.type, offset, entity);
			}

			void ComponentInspector::RenderComponentMember(std::string_view displayName, Reflection::TypeDescriptor* itemType, void* offset, ECS::Entity entity) {
				const char* displayNamePtr = displayName.data();

				switch (itemType->type) {
				case Reflection::TypeDescriptor::ReflectionTypeData::Bool:
					ImGui::Checkbox(
						displayNamePtr,
						(bool*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::AssetReference: {
					GenericAssetReference& assetReference = *(GenericAssetReference*)offset;

					ImGui::Button(
						assetReference.uuid.ToString().c_str()
					);

					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_UUID")) {
							assetReference.asset = payload->Data;
						}
						ImGui::EndDragDropTarget();
					}
					break;
				}
				case Reflection::TypeDescriptor::ReflectionTypeData::String:
					ImGui::InputText(
						displayNamePtr,
						(std::string *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int:
					ImGui::InputInt(
						displayNamePtr,
						(int*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int2:
					ImGui::InputInt2(
						displayNamePtr,
						(int *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int3:
					ImGui::InputInt3(
						displayNamePtr,
						(int *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Int4:
					ImGui::InputInt4(
						displayNamePtr,
						(int *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float:
					ImGui::InputFloat(
						displayNamePtr,
						(float*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float2:
					ImGui::InputFloat2(
						displayNamePtr,
						(float *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float3:
					ImGui::InputFloat3(
						displayNamePtr,
						(float *)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Float4:
					ImGui::InputFloat4(
						displayNamePtr,
						(float*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion: {
					glm::quat* quaternion = (glm::quat*)offset;
					glm::vec3 euler = glm::degrees(glm::eulerAngles(*quaternion));
					if (ImGui::InputFloat3(
						displayNamePtr,
						&euler[0]
					)) {
						*quaternion = glm::quat(glm::radians(euler));
					}
					break;
				}
				case Reflection::TypeDescriptor::ReflectionTypeData::Double:
					ImGui::InputDouble(
						displayNamePtr,
						(double*)offset
					);
					break;
				case Reflection::TypeDescriptor::ReflectionTypeData::Vector:
					ImGui::Text(displayNamePtr);
					ImGui::SameLine();
					const void* vector = static_cast<const void*>(offset);
					auto vectorType = static_cast<Reflection::TypeDescriptor_StdVector*>(itemType);
					size_t vectorSize = vectorType->getSize(offset);

					std::string buttonFieldName = std::string("+##") + displayNamePtr;
					if (ImGui::Button(buttonFieldName.c_str())) {
						vectorType->emplaceBack(offset);
					}

					size_t itemToDelete = -1;
					for (size_t i = 0; i < vectorSize; ++i) {
						std::string fieldName = std::string("##") + std::to_string(i) + displayNamePtr;
						RenderComponentMember(std::string_view(fieldName), vectorType->itemType, vectorType->getItem(offset, i), entity);
						ImGui::SameLine();
						std::string eraseFieldName = std::string("-") + fieldName;
						if (ImGui::Button(eraseFieldName.c_str())) {
							itemToDelete = i;

							// TODO: Handle effects of remove?
						}
					}

					if (itemToDelete != -1) {
						vectorType->erase(offset, itemToDelete);
					}

					break;
				}
			}
		}
	}
}
