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
					if (selection.HasSingleSelectedFile()) {
						materialInspector->Render();
					}
					else if (selection.HasSingleSelectedEntity()) {
						componentInspector->Render(selection.GetSingleSelectedEntity());
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
