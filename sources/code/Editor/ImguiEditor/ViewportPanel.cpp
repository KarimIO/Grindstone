#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include <Common/Input/InputInterface.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/EngineCore.hpp>

#include "../EditorCamera.hpp"
#include "../EditorManager.hpp"
#include "ViewportPanel.hpp"
using namespace Grindstone::Editor::ImguiEditor;

static ImGuizmo::OPERATION ConvertManipulationModeToImGuizmoOperation(Editor::ManipulationMode mode) {
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

	Grindstone::EngineCore& engineCore = Editor::Manager::GetEngineCore();
	auto dispatcher = engineCore.GetEventDispatcher();
	dispatcher->AddEventListener(Grindstone::Events::EventType::MouseButton, std::bind(&ViewportPanel::OnMouseButtonEvent, this, std::placeholders::_1));
	dispatcher->AddEventListener(Grindstone::Events::EventType::MouseMoved, std::bind(&ViewportPanel::OnMouseMovedEvent, this, std::placeholders::_1));
}

bool ViewportPanel::OnMouseButtonEvent(Grindstone::Events::BaseEvent* baseEvent) {
	Grindstone::Events::MousePressEvent* ev = static_cast<Grindstone::Events::MousePressEvent*>(baseEvent);

	// On release right mouse button
	if (isMovingCamera && ev != nullptr && ev->code == Grindstone::Events::MouseButtonCode::Right && !ev->isPressed) {
		isMovingCamera = false;

		Grindstone::EngineCore& engineCore = Editor::Manager::GetEngineCore();
		Grindstone::Input::Interface* input = engineCore.GetInputManager();

		input->SetCursorIsRawMotion(false);
		input->SetCursorMode(Grindstone::Input::CursorMode::Normal);

		return true;
	}

	return false;
}

bool ViewportPanel::OnMouseMovedEvent(Grindstone::Events::BaseEvent* baseEvent) {
	Grindstone::Events::MouseMovedEvent* ev = static_cast<Grindstone::Events::MouseMovedEvent*>(baseEvent);

	// On release right mouse button
	if (isMovingCamera && ev != nullptr) {
		float deltaX = static_cast<float>(ev->mouseX - startDragX);
		float deltaY = static_cast<float>(ev->mouseY - startDragY);
		camera->OffsetRotation(deltaX, deltaY);

		Grindstone::EngineCore& engineCore = Editor::Manager::GetEngineCore();
		Grindstone::Input::Interface* input = engineCore.GetInputManager();
		input->SetMousePosition(startDragX, startDragY);

		return true;
	}

	return false;
}

void ViewportPanel::HandleInput() {
	if (!ImGui::IsWindowHovered()) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	Grindstone::EngineCore& engineCore = Editor::Manager::GetEngineCore();
	Grindstone::Input::Interface* input = engineCore.GetInputManager();

	if (input == nullptr) {
		return;
	}

	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 viewportPanelRegionMin = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportPanelRegionMax = ImGui::GetWindowContentRegionMax();

	unsigned int viewportCenterX = static_cast<unsigned int>((viewportPanelRegionMin.x + viewportPanelRegionMax.x) / 2 + windowPos.x);
	unsigned int viewportCenterY = static_cast<unsigned int>((viewportPanelRegionMin.y + viewportPanelRegionMax.y) / 2 + windowPos.y);

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		auto window = engineCore.windowManager->GetWindowByIndex(0);

		input->GetMousePosition(startDragX, startDragY);

		input->SetCursorIsRawMotion(true);
		input->SetCursorMode(Grindstone::Input::CursorMode::Disabled);
		isMovingCamera = true;
	}

	if (isMovingCamera) {
		bool isWPressed = input->IsKeyPressed(Grindstone::Events::KeyPressCode::W);
		bool isSPressed = input->IsKeyPressed(Grindstone::Events::KeyPressCode::S);

		bool isDPressed = input->IsKeyPressed(Grindstone::Events::KeyPressCode::D);
		bool isAPressed = input->IsKeyPressed(Grindstone::Events::KeyPressCode::A);

		bool isSpacePressed = input->IsKeyPressed(Grindstone::Events::KeyPressCode::Space);
		bool isCtrlPressed = input->IsKeyPressed(Grindstone::Events::KeyPressCode::LeftControl);

		float xOffset = (isAPressed ? 1.f : 0.f) + (isDPressed ? -1.f : 0.f);
		float zOffset = (isWPressed ? 1.f : 0.f) + (isSPressed ? -1.f : 0.f);
		float yOffset = (isSpacePressed ? 1.f : 0.f) + (isCtrlPressed ? -1.f : 0.f);

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

		glm::mat4 transformMatrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
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

		if (ImGuizmo::IsUsing()) {
			ECS::Entity parentEntity = selectedEntity.GetParent();
			glm::mat4 localMatrix;

			if (parentEntity) {
				glm::mat4 parentMatrix = TransformComponent::GetWorldTransformMatrix(parentEntity);
				localMatrix = glm::inverse(parentMatrix) * transformMatrix;
			}
			else {
				localMatrix = transformMatrix;
			}

			glm::vec3 translation, rotation, scale;
			ImGuizmo::DecomposeMatrixToComponents(
				glm::value_ptr(localMatrix),
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

void ViewportPanel::RenderCamera(GraphicsAPI::CommandBuffer* commandBuffer) {
	if (!isShowingPanel || width < 4 || height < 4) {
		return;
	}

	camera->ResizeViewport(width, height);

	PlayMode playMode = Editor::Manager::GetInstance().GetPlayMode();
	if (playMode == PlayMode::Editor) {
		camera->Render(commandBuffer);
	}
	else {
		camera->RenderPlayModeCamera(commandBuffer);
	}
}

void ViewportPanel::DisplayCameraToPanel() {
	uint64_t textureID = camera->GetRenderOutput();

	ImTextureID texturePtr = (ImTextureID)textureID;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	ImVec2 uv0 = ImVec2{ 0, 0 };
	ImVec2 uv1 = ImVec2{ 1, 1 };
	ImGui::Image(texturePtr, viewportPanelSize, uv0, uv1);
}

void ViewportPanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", &isShowingPanel);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		width = std::max((int)viewportPanelSize.x, 1);
		height = std::max((int)viewportPanelSize.y, 1);

		PlayMode playMode = Editor::Manager::GetInstance().GetPlayMode();
		bool isEditorMode = playMode == PlayMode::Editor;
		if (isEditorMode) {
			HandleInput();
		}

		DisplayCameraToPanel();

		if (isEditorMode) {
			HandleSelection();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
