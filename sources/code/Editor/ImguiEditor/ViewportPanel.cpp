#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include "Common/Input.hpp"
#include "EngineCore/CoreComponents/Camera/CameraComponent.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Window/WindowManager.hpp"

#include "../EditorCamera.hpp"
#include "../EditorManager.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
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
		auto window = Editor::Manager::GetEngineCore().windowManager->GetWindowByIndex(0);
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

		auto window = Editor::Manager::GetEngineCore().windowManager->GetWindowByIndex(0);
		window->SetMousePos(viewportCenterX, viewportCenterY);

		camera->OffsetRotation(mouseY, mouseX);
		camera->OffsetPosition(xOffset, yOffset, zOffset);
	}
}

void ViewportPanel::HandleSelection() {
	Selection& selection = Editor::Manager::GetInstance().GetSelection();
	if (selection.GetSelectedEntityCount() == 1 && selection.GetSelectedFileCount() == 0) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		auto pos = ImGui::GetWindowPos();
		ImGuizmo::SetRect(pos.x, pos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		ECS::Entity selectedEntity = selection.GetSingleSelectedEntity();
		TransformComponent* transformComponent = nullptr;
		if (!selectedEntity.TryGetComponent<TransformComponent>(transformComponent)) {
			return;
		}

		glm::mat4 viewMatrix = camera->GetViewMatrix();
		glm::mat4& projectionMatrix = camera->GetProjectionMatrix();
		glm::mat4 transformMatrix = transformComponent->GetTransformMatrix();

		ImGuizmo::Manipulate(
			glm::value_ptr(viewMatrix),
			glm::value_ptr(projectionMatrix),
			ImGuizmo::TRANSLATE,
			ImGuizmo::LOCAL,
			glm::value_ptr(transformMatrix),
			nullptr,
			nullptr
		);

		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale;
			ImGuizmo::DecomposeMatrixToComponents(&transformMatrix[0][0], &translation[0], &rotation[0], &scale[0]);

			transformComponent->position = translation;
			transformComponent->rotation = glm::quat(rotation);
			transformComponent->scale = scale;
		}
	}
}

void ViewportPanel::RenderCamera() {
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	uint32_t width = std::max((int)viewportPanelSize.x, 1);
	uint32_t height = std::max((int)viewportPanelSize.y, 1);
	camera->ResizeViewport(width, height);
	camera->Render();
}

void ViewportPanel::DisplayCameraToPanel() {
	uint64_t textureID = (uint64_t)camera->GetPrimaryFramebufferAttachment();
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
		HandleSelection();

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
