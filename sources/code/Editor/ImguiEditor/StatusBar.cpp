#include <imgui.h>
#include <imgui_internal.h>
#include "StatusBar.hpp"
#include "Editor/EditorManager.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Editor::ImguiEditor;

void StatusBar::Render() {
	ImGuiViewport* viewport = (ImGuiViewport*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	float height = ImGui::GetFrameHeight();

	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyle().Colors[ImGuiCol_CheckMark]);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, windowFlags)) {
		if (ImGui::BeginMenuBar()) {
			ImGui::Text("Ready");
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}
