#include "SceneHeirarchyPanel.hpp"
#include <imgui/imgui.h>

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			void SceneHeirarchyPanel::render() {
				ImGui::Begin("Scene Heirarchy", &isShowingPanel);
				ImGui::Text("This is some useful text.");
				ImGui::End();
			}

			void SceneHeirarchyPanel::renderEntity() {
			}
			
			unsigned int SceneHeirarchyPanel::getSelectedEntity() {
				return selectedEntity;
			}
			
			void SceneHeirarchyPanel::updateSelectedEntity(unsigned int selectedEntity) {
				this->selectedEntity = selectedEntity;
			}
		}
	}
}
