#include <imgui/imgui.h>

#include "Common/Input.hpp"
#include "EngineCore/CoreComponents/Camera/CameraComponent.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Window/WindowManager.hpp"

#include "../EditorCamera.hpp"
#include "../EditorManager.hpp"
#include "ViewportPanel.hpp"
using namespace Grindstone::Editor::ImguiEditor;

ViewportPanel::ViewportPanel() {
	camera = new EditorCamera();
}

void ViewportPanel::HandleInput() {
	if (!ImGui::IsWindowHovered()) {
		return;
	}

	auto& io = ImGui::GetIO();

	ImVec2 viewportPanelPosition = ImVec2(
		(ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x),
		(ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y)
	); 
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	unsigned int viewportCenterX = (unsigned int)(viewportPanelPosition.x + (viewportPanelSize.x / 2.f));
	unsigned int viewportCenterY = (unsigned int)(viewportPanelPosition.y + (viewportPanelSize.y / 2.f));

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		auto window = Editor::Manager::GetInstance().GetEngineCore().windowManager->GetWindowByIndex(0);
		window->SetMousePos(viewportCenterX, viewportCenterY);
	}
	else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		bool isWPressed = io.KeysDown[(int)Grindstone::Events::KeyPressCode::W];
		bool isSPressed = io.KeysDown[(int)Grindstone::Events::KeyPressCode::S];

		bool isDPressed = io.KeysDown[(int)Grindstone::Events::KeyPressCode::D];
		bool isAPressed = io.KeysDown[(int)Grindstone::Events::KeyPressCode::A];

		bool isSpacePressed = io.KeysDown[(int)Grindstone::Events::KeyPressCode::Space];
		bool isCtrlPressed = io.KeyCtrl;

		float xOffset = (isDPressed ? 1.f : 0.f) + (isAPressed ? -1.f : 0.f);
		float zOffset = (isWPressed ? 1.f : 0.f) + (isSPressed ? -1.f : 0.f);
		float yOffset = (isSpacePressed ? 1.f : 0.f) + (isCtrlPressed ? -1.f : 0.f);

		ImVec2 mousePos = ImGui::GetMousePos();
		float mouseX = mousePos.x - (float)viewportCenterX;
		float mouseY = mousePos.y - (float)viewportCenterY;

		auto window = Editor::Manager::GetInstance().GetEngineCore().windowManager->GetWindowByIndex(0);
		window->SetMousePos(viewportCenterX, viewportCenterY);

		camera->OffsetRotation(mouseY, mouseX);
		camera->OffsetPosition(xOffset, yOffset, zOffset);
	}
}

void ViewportPanel::RenderCamera() {
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	camera->ResizeViewport((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
	camera->Render();
}

void ViewportPanel::DisplayCameraToPanel() {
	int textureID = camera->GetPrimaryFramebufferAttachment();
	ImTextureID texturePtr = (ImTextureID)textureID;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	auto uv0 = ImVec2{ 0, 1 };
	auto uv1 = ImVec2{ 1, 0 };
	ImGui::Image(texturePtr, viewportPanelSize, uv0, uv1);
}

void ViewportPanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", &isShowingPanel);

		RenderCamera();
		HandleInput();
		DisplayCameraToPanel();

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
