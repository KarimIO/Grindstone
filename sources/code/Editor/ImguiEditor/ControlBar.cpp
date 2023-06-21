#include <imgui.h>
#include <imgui_internal.h>
#include "ControlBar.hpp"
#include "Editor/EditorManager.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Editor::ImguiEditor;

ControlBar::ControlBar() {

}

void ControlBar::Render() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::Begin("ControlBar", nullptr, flags)) {
		/*
		if (ImGui::Button("Play")) {
		}
		ImGui::SameLine();
		*/

		Editor::Manager& editorManager = Grindstone::Editor::Manager::GetInstance();
		ManipulationMode& manipulationMode = editorManager.manipulationMode;
		if (ImGui::RadioButton("Translate", manipulationMode == ManipulationMode::Translate)) {
			manipulationMode = ManipulationMode::Translate;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Rotate", manipulationMode == ManipulationMode::Rotate)) {
			manipulationMode = ManipulationMode::Rotate;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Scale", manipulationMode == ManipulationMode::Scale)) {
			manipulationMode = ManipulationMode::Scale;
		}
		ImGui::SameLine();

		bool& isManipulatingInWorldSpace = editorManager.isManipulatingInWorldSpace;
		const char* worldLocalToggleString = isManipulatingInWorldSpace ? "World" : "Local";
		if (ImGui::Button(worldLocalToggleString)) {
			isManipulatingInWorldSpace = !isManipulatingInWorldSpace;
		}

		ImGui::End();
	}
}
