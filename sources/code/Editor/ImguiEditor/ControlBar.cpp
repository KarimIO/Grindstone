#include <imgui.h>
#include <imgui_internal.h>
#include "ControlBar.hpp"
#include "ImguiRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Editor::ImguiEditor;

ControlBar::ControlBar(ImguiRenderer* imguiRenderer) {
	pauseIcon = imguiRenderer->CreateTexture("controlbarIcons/PauseButton.png");
	playIcon = imguiRenderer->CreateTexture("controlbarIcons/PlayButton.png");
	translateIcon = imguiRenderer->CreateTexture("controlbarIcons/Translate.png");
	rotateIcon = imguiRenderer->CreateTexture("controlbarIcons/Rotate.png");
	scaleIcon = imguiRenderer->CreateTexture("controlbarIcons/Scale.png");
}

void ControlBar::Render() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::Begin("ControlBar", nullptr, flags)) {
		Editor::Manager& editorManager = Grindstone::Editor::Manager::GetInstance();

		float centerX = ImGui::GetWindowContentRegionMax().x / 2.0f - 12.0f;

		selectedColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		selectedHighlightColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		selectedActiveColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

		deselectedColor = ImVec4(1.f, 1.f, 1.f, 0.1f);
		deselectedHighlightColor = ImVec4(1.f, 1.f, 1.f, 0.2f);
		deselectedActiveColor = ImVec4(1.f, 1.f, 1.f, 0.3f);

		ManipulationMode& manipulationMode = editorManager.manipulationMode;
		RenderManipulationButton(translateIcon, manipulationMode, ManipulationMode::Translate);
		ImGui::SameLine();
		RenderManipulationButton(rotateIcon, manipulationMode, ManipulationMode::Rotate);
		ImGui::SameLine();
		RenderManipulationButton(scaleIcon, manipulationMode, ManipulationMode::Scale);
		ImGui::SameLine();

		bool& isManipulatingInWorldSpace = editorManager.isManipulatingInWorldSpace;
		const char* worldLocalToggleString = isManipulatingInWorldSpace ? "World" : "Local";
		if (ImGui::Button(worldLocalToggleString)) {
			isManipulatingInWorldSpace = !isManipulatingInWorldSpace;
		}

		ImGui::SameLine();

		ImGui::SetCursorPosX(centerX);
		PlayMode playMode = editorManager.GetPlayMode();
		bool isPlayMode = playMode == PlayMode::Play;
		if (RenderButton(playIcon, isPlayMode)) {
			PlayMode newMode = isPlayMode ? PlayMode::Editor : PlayMode::Play;
			editorManager.SetPlayMode(newMode);
		}

		ImGui::End();
	}
}

bool ControlBar::RenderButton(ImTextureID icon, bool isSelected) {
	ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? selectedColor : deselectedColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, isSelected ? selectedActiveColor : deselectedActiveColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, isSelected ? selectedHighlightColor : deselectedHighlightColor);

	int isPressed = ImGui::ImageButton(icon, ImVec2(16, 16), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.f, 1.f, 1.f, 1.f));

	ImGui::PopStyleColor(3);

	return isPressed;
}

void ControlBar::RenderManipulationButton(ImTextureID icon, ManipulationMode& selectedMode, ManipulationMode buttonMode) {
	if (RenderButton(icon, buttonMode == selectedMode)) {
		selectedMode = buttonMode;
	}
}
