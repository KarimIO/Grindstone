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
	auto& io = ImGui::GetIO();

	ImVec2 viewportPanelPosition = ImVec2(
		(ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x),
		(ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y)
	); 
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	ImVec2 viewportCenter = ImVec2(
		(int)(viewportPanelPosition.x + (viewportPanelSize.x / 2.f)),
		(int)(viewportPanelPosition.y + (viewportPanelSize.y / 2.f))
	);

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		auto window = Editor::Manager::GetInstance().GetEngineCore().windowManager->GetWindowByIndex(0);
		window->SetMousePos(viewportCenter.x, viewportCenter.y);
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
		float mouseX = mousePos.x - viewportCenter.x;
		float mouseY = mousePos.y - viewportCenter.y;

		auto window = Editor::Manager::GetInstance().GetEngineCore().windowManager->GetWindowByIndex(0);
		window->SetMousePos(viewportCenter.x, viewportCenter.y);

		camera->OffsetRotation(mouseY, mouseX);
		camera->OffsetPosition(xOffset, yOffset, zOffset);
	}
}

void ViewportPanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", &isShowingPanel);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		camera->ResizeViewport(viewportPanelSize.x, viewportPanelSize.y);
		HandleInput();
		camera->Render();

		int textureID = camera->GetPrimaryFramebufferAttachment();
		auto texturePtr = reinterpret_cast<void*>(textureID);
		auto size = ImVec2{ viewportPanelSize.x, viewportPanelSize.y };
		auto uv0 = ImVec2{ 0, 1 };
		auto uv1 = ImVec2{ 1, 0 };
		ImGui::Image(texturePtr, size, uv0, uv1);

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
