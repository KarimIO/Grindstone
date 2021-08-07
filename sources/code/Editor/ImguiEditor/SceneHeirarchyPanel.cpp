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
			
			void SceneHeirarchyPanel::render() {
				if (isShowingPanel) {
					ImGui::Begin("Scene Heirarchy", &isShowingPanel);

					auto numScenes = sceneManager->scenes.size();
					if (numScenes == 0) {
						ImGui::Text("No entities in this scene.");
					}
					else if (numScenes == 1) {
						auto sceneIterator = sceneManager->scenes.begin();
						renderScene(sceneIterator->second);
					}
					else {
						for (auto& scenePair : sceneManager->scenes) {
							auto* scene = scenePair.second;
							const char* sceneName = scene->getName();
							if (ImGui::TreeNode(sceneName)) {
								renderScene(scene);
								ImGui::TreePop();
							}
						}
					}
					
					if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
						editor->selectEntity(entt::null);
					}

					ImGui::End();
				}
			}

			const char* SceneHeirarchyPanel::getEntityTag(entt::registry& registry, entt::entity entity) {
				if (registry.has<TagComponent>(entity)) {
					TagComponent& tag = registry.get<TagComponent>(entity);
					return tag.tag.c_str();
				}

				return "[Unnamed Entity]";
			}

			void SceneHeirarchyPanel::renderScene(SceneManagement::Scene* scene) {
				auto& registry = *scene->getEntityRegistry();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.1f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.15f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f));
				registry.each(
					[&](auto entity) {
						renderEntity(registry, entity);
					}
				);
				ImGui::PopStyleColor(3);

				if (ImGui::BeginPopupContextWindow(0, RIGHT_MOUSE_BUTTON, false)) {
					if (ImGui::MenuItem("Add new entity")) {
						Editor::Manager::GetInstance().getCommandList().AddNewEntity(scene);
					}
					ImGui::EndPopup();
				}
			}

			void SceneHeirarchyPanel::renderEntity(entt::registry& registry, entt::entity entity) {
				const float panelWidth = ImGui::GetContentRegionAvail().x;
				ImGui::PushItemWidth(panelWidth);

				const char* entityTag = getEntityTag(registry, entity);
				if (entityToRename == entity) {
					const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
					if (ImGui::InputText("##AssetRename", &entityRenameNewName, flags)) {
						entityToRename = entt::null;
						if (registry.has<TagComponent>(entity)) {
							TagComponent& tag = registry.get<TagComponent>(entity);
							tag.tag = entityRenameNewName;
						}
						entityRenameNewName = "";
					}
				}
				else {
					ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0, 0.5});
					if (ImGui::Button(entityTag, {panelWidth, 0})) {
						editor->selectEntity(entity);
					}
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Rename")) {
							entityToRename = entity;
							entityRenameNewName = entityTag;
						}
						if (ImGui::MenuItem("Delete")) {
							editor->selectEntity(entt::null);
							registry.destroy(entity);
						}
						ImGui::EndPopup();
					}
					ImGui::PopStyleVar();
				}
			}
		}
	}
}
