#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "InspectorPanel.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/EngineCore.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			InspectorPanel::InspectorPanel(EngineCore* engineCore) : engineCore(engineCore) {
				componentInspector = new ComponentInspector();
			}
			
			void InspectorPanel::render() {
				if (ImGui::Begin("Inspector", &isShowingPanel)) {
					
					auto sceneManager = engineCore->getSceneManager();
					auto numScenes = sceneManager->scenes.size();
					if (numScenes == 0) {
						ImGui::Text("No entities in this scene.");
					}
					else {
						auto sceneIterator = sceneManager->scenes.begin();
						SceneManagement::Scene* scene = sceneIterator->second;
						auto& registrar = *scene->getComponentRegistrar();
						auto& registry = *scene->getEntityRegistry();
						auto selectedEntity = entt::entity(0);
						componentInspector->render(registrar, registry, selectedEntity);
					}

					ImGui::End();
				}
			}
		}
	}
}
