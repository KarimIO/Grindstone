#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "MaterialInspector.hpp"
#include "InspectorPanel.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/EngineCore.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			InspectorPanel::InspectorPanel(EngineCore* engineCore) : engineCore(engineCore) {
				componentInspector = new ComponentInspector();
				materialInspector = new MaterialInspector();
			}

			void InspectorPanel::selectFile(std::string selectedFileType, std::string selectedFilePath) {
				this->selectedFileType = selectedFileType;
				this->selectedFilePath = selectedFilePath;
				selectedEntity = entt::null;

				if (selectedFileType == "material") {
					materialInspector->setMaterialPath(selectedFilePath.c_str());
				}
			}

			void InspectorPanel::selectEntity(entt::entity selectedEntity) {
				this->selectedEntity = selectedEntity;
				selectedFileType = "";
				selectedFilePath = "";
			}
			
			void InspectorPanel::render() {
				if (isShowingPanel) {
					ImGui::Begin("Inspector", &isShowingPanel);

					if (selectedFileType != "") {
						materialInspector->render();
					}
					else {
						auto sceneManager = engineCore->getSceneManager();
						auto numScenes = sceneManager->scenes.size();
						if (numScenes == 0) {
							ImGui::Text("No entities in this scene.");
						}
						else if (selectedEntity != entt::null) {
							auto sceneIterator = sceneManager->scenes.begin();
							SceneManagement::Scene* scene = sceneIterator->second;
							auto& registrar = *scene->getComponentRegistrar();
							auto& registry = *scene->getEntityRegistry();
							componentInspector->render(registrar, registry, selectedEntity);
						}
						else {
							ImGui::Text("No entity selected.");
						}
					}

					ImGui::End();
				}
			}
		}
	}
}
