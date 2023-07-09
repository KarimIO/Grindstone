#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include "Common/Input.hpp"
#include "EngineCore/CoreComponents/Camera/CameraComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Window/WindowManager.hpp"

#include "../EditorCamera.hpp"
#include "../EditorManager.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "ViewportPanel.hpp"
using namespace Grindstone::Editor::ImguiEditor;

ImGuizmo::OPERATION ConvertManipulationModeToImGuizmoOperation(Editor::ManipulationMode mode) {
	switch (mode) {
	case Editor::ManipulationMode::Rotate:
		return ImGuizmo::ROTATE;
	case Editor::ManipulationMode::Scale:
		return ImGuizmo::SCALE;
	default:
		return ImGuizmo::TRANSLATE;
	}
}

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
	auto& editorManager = Editor::Manager::GetInstance();
	Selection& selection = editorManager.GetSelection();
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

		glm::mat4 transformMatrix = transformComponent->GetTransformMatrix();
		glm::mat4& viewMatrix = camera->GetViewMatrix();
		glm::mat4& projectionMatrix = camera->GetProjectionMatrix();

		ImGuizmo::Manipulate(
			glm::value_ptr(viewMatrix),
			glm::value_ptr(projectionMatrix),
			ConvertManipulationModeToImGuizmoOperation(editorManager.manipulationMode),
			editorManager.isManipulatingInWorldSpace ? ImGuizmo::WORLD : ImGuizmo::LOCAL,
			glm::value_ptr(transformMatrix),
			nullptr,
			nullptr
		);

		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale;
			ImGuizmo::DecomposeMatrixToComponents(
				glm::value_ptr(transformMatrix),
				glm::value_ptr(translation),
				glm::value_ptr(rotation),
				glm::value_ptr(scale)
			);

			rotation *= glm::pi<float>() / 180.0f;
			glm::vec3 originalEuler = glm::eulerAngles(transformComponent->rotation);
			glm::vec3 deltaRotation = rotation - originalEuler;
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

void ViewportPanel::DisplayCameraToPanel(uint64_t textureID) {
	ImTextureID texturePtr = (ImTextureID)textureID;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	auto uv0 = ImVec2{ 0, 1 };
	auto uv1 = ImVec2{ 1, 0 };
	ImGui::Image(texturePtr, viewportPanelSize, uv0, uv1);
}

void ViewportPanel::DisplayInGameCamera() {
	auto sceneManager = Editor::Manager::GetEngineCore().GetSceneManager();

	if (sceneManager == nullptr) {
		return;
	}

	if (sceneManager->scenes.size() == 0) {
		return;
	}

	entt::registry& registry = sceneManager->scenes.begin()->second->GetEntityRegistry();
	TransformComponent* transformComponent = nullptr;
	CameraComponent* cameraComponent = nullptr;
	auto view = registry.view<TransformComponent, CameraComponent>();
	view.each(
		[&](
			TransformComponent& currentTransform,
			CameraComponent& currentCamera
		) {
			transformComponent = &currentTransform;
			cameraComponent = &currentCamera;
		}
	);

	if (cameraComponent == nullptr || cameraComponent->renderer == nullptr) {
		return;
	}

	camera->RenderPlayModeCamera(*transformComponent, *cameraComponent);

	//uint64_t textureID = (uint64_t)camera->GetPrimaryFramebufferAttachment();
	//DisplayCameraToPanel(textureID);
}

void ViewportPanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", &isShowingPanel);

		PlayMode playMode = Editor::Manager::GetInstance().GetPlayMode();
		if (playMode == PlayMode::Editor) {
			HandleInput();
			RenderCamera();
			// uint64_t textureID = (uint64_t)camera->GetPrimaryFramebufferAttachment();
			//DisplayCameraToPanel(0);
			HandleSelection();
		}
		else {
			DisplayInGameCamera();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
