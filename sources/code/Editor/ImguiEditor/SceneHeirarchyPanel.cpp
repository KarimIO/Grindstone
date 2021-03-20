#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "SceneHeirarchyPanel.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			SceneHeirarchyPanel::SceneHeirarchyPanel(SceneManagement::SceneManager* sceneManager) {
				this->sceneManager = sceneManager;
			}
			
			void SceneHeirarchyPanel::render() {
				ImGui::Begin("Scene Heirarchy", &isShowingPanel);

				auto numScenes = sceneManager->scenes.size();
				if (numScenes == 0) {}
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

				ImGui::End();
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
				
				if (selectedEntity != entt::null) {
					const char* selectedEntityTag = getEntityTag(registry, selectedEntity);
					ImGui::Text(selectedEntityTag);
				}

				registry.each(
					[&](auto entity) {
						renderEntity(registry, entity);
					}
				);
			}

			void SceneHeirarchyPanel::renderEntity(entt::registry& registry, entt::entity entity) {
				const char* entityTag = getEntityTag(registry, entity);
				if (ImGui::Button(entityTag)) {
					updateSelectedEntity(entity);
				}
			}
			
			entt::entity SceneHeirarchyPanel::getSelectedEntity() {
				return selectedEntity;
			}
			
			void SceneHeirarchyPanel::updateSelectedEntity(entt::entity selectedEntity) {
				this->selectedEntity = selectedEntity;
			}
		}
	}
}
