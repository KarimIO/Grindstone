#include <imgui.h>
#include <imgui_internal.h>
#include "ControlBar.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Textures/TextureAsset.hpp"
#include "Plugins/GraphicsOpenGL/GLTexture.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Editor::ImguiEditor;

ControlBar::ControlBar() {
	pauseIcon = GetTexture("PauseButton.dds");
	playIcon = GetTexture("PlayButton.dds");
	translateIcon = GetTexture("Translate.dds");
	rotateIcon = GetTexture("Rotate.dds");
	scaleIcon = GetTexture("Scale.dds");
}

void ControlBar::Render() {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::Begin("ControlBar", nullptr, flags)) {
		Editor::Manager& editorManager = Grindstone::Editor::Manager::GetInstance();

		float centerX = ImGui::GetWindowContentRegionWidth() / 2.0f - 12.0f;

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

ImTextureID ControlBar::GetTexture(std::string fileName) {
	std::string path = std::string("../engineassets/editor/controlbarIcons/") + fileName;
	auto assetManager = Editor::Manager::GetEngineCore().assetManager;
	auto textureAsset = static_cast<TextureAsset*>(assetManager->GetAsset(Grindstone::AssetType::Texture, path.c_str()));

	if (textureAsset == nullptr) {
		return 0;
	}

	GraphicsAPI::GLTexture* glTex = (GraphicsAPI::GLTexture*)textureAsset->texture;
	return (ImTextureID)(uint64_t)glTex->GetTexture();
}
