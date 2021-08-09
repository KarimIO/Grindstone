#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <entt/entt.hpp>
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "Editor/Commands/EntityCommands.hpp"
#include "Editor/EditorManager.hpp"
#include "ImguiEditor.hpp"
#include "SceneHeirarchyPanel.hpp"

const bool RIGHT_MOUSE_BUTTON = 1;

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			SceneHeirarchyPanel::SceneHeirarchyPanel(
				SceneManagement::SceneManager* sceneManager,
				ImguiEditor* editor
			) : sceneManager(sceneManager), editor(editor) {}
			
			void SceneHeirarchyPanel::Render() {
				if (isShowingPanel) {
					ImGui::Begin("Scene Heirarchy", &isShowingPanel);

					if (
						ImGui::IsMouseDown(0) &&
						ImGui::IsWindowHovered() &&
						!ImGui::GetIO().KeyShift
						) {
						Editor::Manager::GetInstance().GetSelection().Clear();
					}

					auto numScenes = sceneManager->scenes.size();
					if (numScenes == 0) {
						ImGui::Text("No scenes mounted.");
					}
					else if (numScenes == 1) {
						auto sceneIterator = sceneManager->scenes.begin();
						RenderScene(sceneIterator->second);
					}
					else {
						for (auto& scenePair : sceneManager->scenes) {
							auto* scene = scenePair.second;
							const char* sceneName = scene->GetName();
							if (ImGui::TreeNode(sceneName)) {
								RenderScene(scene);
								ImGui::TreePop();
							}
						}
					}

					ImGui::End();
				}
			}

			const char* SceneHeirarchyPanel::GetEntityTag(ECS::Entity entity) {
				if (entity.HasComponent<TagComponent>()) {
					return entity.GetComponent<TagComponent>().tag.c_str();
				}

				return "[Unnamed Entity]";
			}

			void SceneHeirarchyPanel::RenderScene(SceneManagement::Scene* scene) {
				auto& registry = scene->GetEntityRegistry();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.1f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.15f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f));
				registry.each(
					[&](auto entity) {
						RenderEntity({ entity, scene });
					}
				);
				ImGui::PopStyleColor(3);

				if (ImGui::BeginPopupContextWindow(0, RIGHT_MOUSE_BUTTON, false)) {
					if (ImGui::MenuItem("Add new entity")) {
						Editor::Manager::GetInstance().GetCommandList().AddNewEntity(scene);
					}
					ImGui::EndPopup();
				}
			}

			void SceneHeirarchyPanel::RenderEntity(ECS::Entity entity) {
				const float panelWidth = ImGui::GetContentRegionAvail().x;
				ImGui::PushItemWidth(panelWidth);

				const char* entityTag = GetEntityTag(entity);
				if (entityToRename == entity) {
					const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
					if (ImGui::InputText("##AssetRename", &entityRenameNewName, flags)) {
						entityToRename = ECS::Entity();
						TagComponent* tagComponent = nullptr;
						if (entity.TryGetComponent<TagComponent>(tagComponent)) {
							tagComponent->tag = entityRenameNewName;
						}
						entityRenameNewName = "";
					}
				}
				else {
					Selection& selection = Editor::Manager::GetInstance().GetSelection();
					ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0, 0.5});
					if (ImGui::Button(entityTag, {panelWidth, 0})) {
						if (ImGui::GetIO().KeyShift) {
							selection.AddEntity(entity);
						}
						else {
							selection.SetSelectedEntity(entity);
						}
					}
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Rename")) {
							entityToRename = entity;
							entityRenameNewName = entityTag;
						}
						if (ImGui::MenuItem("Delete")) {
							selection.RemoveEntity(entity);
							entity.GetScene()->DestroyEntity(entity);
						}
						ImGui::EndPopup();
					}
					ImGui::PopStyleVar();
				}
			}
		}
	}
}
