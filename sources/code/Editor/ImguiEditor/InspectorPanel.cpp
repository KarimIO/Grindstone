#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "MaterialInspector.hpp"
#include "InspectorPanel.hpp"
#include "Editor/EditorManager.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/EngineCore.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			InspectorPanel::InspectorPanel(EngineCore* engineCore) : engineCore(engineCore) {
				componentInspector = new ComponentInspector();
				materialInspector = new MaterialInspector(engineCore);
			}

			void InspectorPanel::Render() {
				if (isShowingPanel) {
					ImGui::Begin("Inspector", &isShowingPanel);
					
					Selection& selection = Editor::Manager::GetInstance().GetSelection();
					size_t selectedEntityCount = selection.GetSelectedEntityCount();
					size_t selectedFileCount = selection.GetSelectedFileCount();
					if (selectedEntityCount == 0 && selectedFileCount == 1) {
						materialInspector->Render();
					}
					else if (selectedEntityCount == 1 && selectedFileCount == 0) {
						componentInspector->Render(selection.GetSingleSelectedEntity());
					}
					else if (selectedEntityCount > 0 || selectedFileCount > 0) {
						if (selectedEntityCount > 0) {
							ImGui::Text("%i entities selected.", selectedEntityCount);
						}

						if (selectedFileCount > 0) {
							ImGui::Text("%i files selected.", selectedFileCount);
						}
					}
					else {
						ImGui::Text("Nothing selected.");
					}

					ImGui::End();
				}
			}
		}
	}
}
