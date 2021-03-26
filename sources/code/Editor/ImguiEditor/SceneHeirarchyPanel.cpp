#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
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
						editor->updateSelectedEntity(entt::null);
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

				registry.each(
					[&](auto entity) {
						renderEntity(registry, entity);
					}
				);

				if (ImGui::BeginPopupContextWindow(0, RIGHT_MOUSE_BUTTON, false)) {
					if (ImGui::MenuItem("Add new entity")) {
						scene->createDefaultEntity();
					}
					ImGui::EndPopup();
				}
			}

			void SceneHeirarchyPanel::renderEntity(entt::registry& registry, entt::entity entity) {
				const char* entityTag = getEntityTag(registry, entity);
				if (ImGui::Button(entityTag)) {
					editor->updateSelectedEntity(entity);
				}
			}
		}
	}
}
