#include <Common/Display/DisplayManager.hpp>
#include <Common/Graphics/Framebuffer.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Plugins/GraphicsVulkan/VulkanDescriptorSet.hpp>
#include <Plugins/PhysicsBullet/Components/ColliderComponent.hpp>
#include <Plugins/Renderables3D/Components/MeshComponent.hpp>

#include "EditorCamera.hpp"
#include "EditorManager.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone::Editor;
using namespace Grindstone;

const Grindstone::ConstHashedString editorRenderPassHashedString("Editor");
const Grindstone::ConstHashedString gizmoRenderPassHashedString("Gizmo");

EditorCamera::EditorCamera() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	Grindstone::RenderPassRegistry* renderPassRegistry = engineCore.GetRenderPassRegistry();
	GraphicsAPI::Core* core = engineCore.GetGraphicsCore();

	Display display = engineCore.displayManager->GetMainDisplay();
	uint32_t framebufferWidth = display.width;
	uint32_t framebufferHeight = display.height;

	GraphicsAPI::Image::CreateInfo renderTargetCreateInfo{};
	renderTargetCreateInfo.debugName = "Editor Viewport Color Image";
	renderTargetCreateInfo.width = framebufferWidth;
	renderTargetCreateInfo.height = framebufferHeight;
	renderTargetCreateInfo.format = GraphicsAPI::Format::R8G8B8A8_UNORM;
	renderTargetCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::Sampled |
		GraphicsAPI::ImageUsageFlags::RenderTarget;
	renderTarget = core->CreateImage(renderTargetCreateInfo);

	GraphicsAPI::Image::CreateInfo depthTargetCreateInfo{};
	depthTargetCreateInfo.debugName = "Editor Viewport Depth Image";
	depthTargetCreateInfo.width = framebufferWidth;
	depthTargetCreateInfo.height = framebufferHeight;
	depthTargetCreateInfo.format = GraphicsAPI::Format::D32_SFLOAT;
	depthTargetCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::TransferDst |
		GraphicsAPI::ImageUsageFlags::Sampled |
		GraphicsAPI::ImageUsageFlags::DepthStencil;
	depthTarget = core->CreateImage(depthTargetCreateInfo);

	std::array<GraphicsAPI::RenderPass::AttachmentInfo, 1> attachments = { { renderTargetCreateInfo.format, true } };

	GraphicsAPI::RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Editor RenderPass";
	renderPassCreateInfo.colorAttachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.colorAttachments = attachments.data();
	renderPassCreateInfo.depthFormat = GraphicsAPI::Format::D32_SFLOAT;
	renderPassCreateInfo.shouldClearDepthOnLoad = true;
	renderPass = core->CreateRenderPass(renderPassCreateInfo);
	renderPassRegistry->RegisterRenderpass(editorRenderPassHashedString, renderPass);

	std::array<GraphicsAPI::RenderPass::AttachmentInfo, 1> gizmoAttachments = { { renderTargetCreateInfo.format, false } };

	GraphicsAPI::RenderPass::CreateInfo gizmoRenderPassCreateInfo{};
	gizmoRenderPassCreateInfo.debugName = "Editor Gizmo RenderPass";
	gizmoRenderPassCreateInfo.colorAttachmentCount = static_cast<uint32_t>(gizmoAttachments.size());
	gizmoRenderPassCreateInfo.colorAttachments = gizmoAttachments.data();
	gizmoRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::D32_SFLOAT;
	gizmoRenderPassCreateInfo.shouldClearDepthOnLoad = false;
	gizmoRenderPass = core->CreateRenderPass(gizmoRenderPassCreateInfo);
	renderPassRegistry->RegisterRenderpass(gizmoRenderPassHashedString, gizmoRenderPass);

	GraphicsAPI::Framebuffer::CreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.debugName = "Editor Framebuffer";
	framebufferCreateInfo.renderTargets = &renderTarget;
	framebufferCreateInfo.renderTargetCount = 1;
	framebufferCreateInfo.depthTarget = depthTarget;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.width = framebufferWidth;
	framebufferCreateInfo.height = framebufferHeight;
	framebuffer = core->CreateFramebuffer(framebufferCreateInfo);

	Grindstone::GraphicsAPI::Sampler::CreateInfo samplerCreateInfo;
	samplerCreateInfo.debugName = "Editor Sampler";
	samplerCreateInfo.options.anistropy = 16.0f;
	samplerCreateInfo.options.mipMin = -1000.0f;
	samplerCreateInfo.options.mipMax = 1000.0f;
	samplerCreateInfo.options.mipFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Linear;
	samplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::ClampToEdge;
	samplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::ClampToEdge;
	samplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::ClampToEdge;
	sampler = core->CreateSampler(samplerCreateInfo);

	GraphicsAPI::DescriptorSetLayout::Binding descriptorSetLayoutBinding{};
	descriptorSetLayoutBinding.bindingId = 0;
	descriptorSetLayoutBinding.type = GraphicsAPI::BindingType::CombinedImageSampler;
	descriptorSetLayoutBinding.count = 1;
	descriptorSetLayoutBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.debugName = "Editor Viewport Descriptor Set Layout";
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.bindings = &descriptorSetLayoutBinding;
	descriptorSetLayout = core->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	std::pair<GraphicsAPI::Image*, GraphicsAPI::Sampler*> combinedSamplerPair = { renderTarget, sampler };
	GraphicsAPI::DescriptorSet::Binding descriptorSetBinding = GraphicsAPI::DescriptorSet::Binding::CombinedImageSampler( &combinedSamplerPair );

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "Editor Viewport Descriptor Set";
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.bindings = &descriptorSetBinding;
	descriptorSetCreateInfo.layout = descriptorSetLayout;
	descriptorSet = core->CreateDescriptorSet(descriptorSetCreateInfo);

	gridRenderer.Initialize(renderPass);
	gizmoRenderer.Initialize(renderPass);

	renderer = engineCore.GetRendererFactory()->CreateRenderer(renderPass);
	UpdateViewMatrix();
}

EditorCamera::~EditorCamera() {
	AllocatorCore::Free(renderer);
	renderer = nullptr;
}

uint64_t EditorCamera::GetRenderOutput() {
	return (uint64_t)(static_cast<GraphicsAPI::Vulkan::DescriptorSet*>(descriptorSet)->GetDescriptorSet());
}

void EditorCamera::Render(GraphicsAPI::CommandBuffer* commandBuffer) {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	EngineCore& engineCore = editorManager.GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	SceneManagement::SceneManager* sceneManager = engineCore.GetSceneManager();

	if (sceneManager == nullptr) {
		return;
	}

	if (sceneManager->scenes.size() == 0) {
		return;
	}

	SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;

	if (scene == nullptr) {
		return;
	}

	entt::registry& registry = scene->GetEntityRegistry();
	renderer->Render(
		commandBuffer,
		registry,
		projection,
		view,
		position,
		framebuffer
	);

	glm::mat4 gizmoProjection = projection;
	graphicsCore->AdjustPerspective(&gizmoProjection[0][0]);
	glm::mat4 projView = gizmoProjection * view;
	glm::vec2 renderScale = glm::vec2(
		static_cast<float>(width) / framebuffer->GetWidth(),
		static_cast<float>(height) / framebuffer->GetHeight()
	);

	Grindstone::GraphicsAPI::ClearColor clearColor{};
	Grindstone::GraphicsAPI::ClearDepthStencil clearDepthStencil;
	clearDepthStencil.hasDepthStencilAttachment = true;
	commandBuffer->BindRenderPass(gizmoRenderPass, framebuffer, width, height, &clearColor, 1, clearDepthStencil);
	if (isGridEnabled) {
		gridRenderer.Render(commandBuffer, renderScale, gizmoProjection, view, nearPlaneDistance, farPlaneDistance, glm::quat(), 0.0f);
	}

	if (editorManager.GetSelection().GetSelectedEntityCount() > 0) {
		static const glm::vec4 boundingBoxColor = glm::vec4(0.2f, 0.9f, 0.3f, 1.0f);
		static const glm::vec4 boundingSphereColor = glm::vec4(0.2f, 0.9f, 0.3f, 0.4f);
		static const glm::vec4 colliderColor = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);

		Physics::BoxColliderComponent* box = nullptr;
		Physics::CapsuleColliderComponent* capsule = nullptr;
		Physics::PlaneColliderComponent* plane = nullptr;
		Physics::SphereColliderComponent* sphere = nullptr;
		Grindstone::MeshComponent* mesh = nullptr;

		for (const ECS::Entity& selectedEntity : editorManager.GetSelection().selectedEntities) {
			if (
				(isBoundingSphereGizmoEnabled || isBoundingBoxGizmoEnabled) &&
				selectedEntity.TryGetComponent<Grindstone::MeshComponent>(mesh)
			) {
				Grindstone::Mesh3dAsset* meshAsset = engineCore.assetManager->GetAssetByUuid<Grindstone::Mesh3dAsset>(mesh->mesh.uuid);
				auto& boundingData = meshAsset->boundingData;
				TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
				Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
				glm::vec3 center = boundingData.sphereCenter;
				glm::vec3 boxSize = boundingData.maxAABB - boundingData.minAABB;
				matrix = matrix * glm::translate(center);
				if (isBoundingSphereGizmoEnabled) {
					gizmoRenderer.SubmitSphereGizmo(matrix, boundingData.sphereRadius, boundingSphereColor);
				}

				if (isBoundingBoxGizmoEnabled) {
					gizmoRenderer.SubmitCubeGizmo(matrix, boxSize, boundingBoxColor);
				}
			}

			if (isColliderGizmoEnabled) {
				if (selectedEntity.TryGetComponent<Physics::BoxColliderComponent>(box)) {
					TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
					Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
					gizmoRenderer.SubmitCubeGizmo(matrix, box->GetSize(), colliderColor);
				}
				else if (selectedEntity.TryGetComponent<Physics::CapsuleColliderComponent>(capsule)) {
					TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
					Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
					gizmoRenderer.SubmitCapsuleGizmo(matrix, capsule->GetHeight(), capsule->GetRadius(), colliderColor);
				}
				else if (selectedEntity.TryGetComponent<Physics::PlaneColliderComponent>(plane)) {
					TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
					Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
					gizmoRenderer.SubmitPlaneGizmo(matrix, plane->GetPlaneNormal(), plane->GetPositionAlongNormal(), colliderColor);
				}
				else if (selectedEntity.TryGetComponent<Physics::SphereColliderComponent>(sphere)) {
					TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
					Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
					gizmoRenderer.SubmitSphereGizmo(matrix, sphere->GetRadius(), colliderColor);
				}
			}
		}

		gizmoRenderer.Render(commandBuffer, projView);
	}
	commandBuffer->UnbindRenderPass();
}

void EditorCamera::RenderPlayModeCamera(GraphicsAPI::CommandBuffer* commandBuffer) {
	EngineCore& engineCore = Editor::Manager::GetInstance().GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	SceneManagement::SceneManager* sceneManager = engineCore.GetSceneManager();

	if (sceneManager == nullptr) {
		return;
	}

	if (sceneManager->scenes.size() == 0) {
		return;
	}

	SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;

	if (scene == nullptr) {
		return;
	}

	entt::registry& registry = scene->GetEntityRegistry();
	entt::entity entity = entt::null;
	const TransformComponent* transformComponent = nullptr;
	CameraComponent* cameraComponent = nullptr;
	auto view = registry.view<entt::entity, const TransformComponent, CameraComponent>();
	view.each(
		[&](
			entt::entity currentEntity,
			const TransformComponent& currentTransform,
			CameraComponent& currentCamera
		) {
			entity = currentEntity;
			transformComponent = &currentTransform;
			cameraComponent = &currentCamera;
		}
	);

	if (entity == entt::null || cameraComponent == nullptr || cameraComponent->renderer == nullptr) {
		return;
	}

	cameraComponent->aspectRatio = static_cast<float>(width) / height;
	cameraComponent->renderer->Resize(width, height);

	const glm::mat4 transformMatrix = TransformComponent::GetWorldTransformMatrix(entity, registry);

	const glm::vec3 upVector = glm::normalize(-glm::vec3(transformMatrix[1]));
	const glm::vec3 forwardVector = glm::normalize(glm::vec3(transformMatrix[2]));
	const glm::vec3 pos = glm::vec3(transformMatrix[3]);

	const glm::mat4 viewMatrix = glm::lookAt(
		pos,
		pos + forwardVector,
		upVector
	);
	
	const glm::mat4 projectionMatrix = glm::perspective(
		cameraComponent->fieldOfView,
		cameraComponent->aspectRatio,
		cameraComponent->nearPlaneDistance,
		cameraComponent->farPlaneDistance
	);

	renderer->Render(
		commandBuffer,
		registry,
		projectionMatrix,
		viewMatrix,
		pos,
		framebuffer
	);
}

const float maxAngle = 1.55f;
void EditorCamera::OffsetRotation(float xOffset, float yOffset) {
	float deltaTime = (float)Editor::Manager::GetEngineCore().GetDeltaTime();

	const float mouseSensitivity = 30.0f;
	eulerAngles.x += yOffset * mouseSensitivity * deltaTime / width;
	eulerAngles.y -= xOffset * mouseSensitivity * deltaTime / height;

	if (eulerAngles.x < -maxAngle) {
		eulerAngles.x = -maxAngle;
	}
	else if (eulerAngles.x > maxAngle) {
		eulerAngles.x = maxAngle;
	}

	rotation = glm::quat(eulerAngles);
	UpdateViewMatrix();
}

void EditorCamera::OffsetPosition(float x, float y, float z) {
	float deltaTime = (float)Editor::Manager::GetEngineCore().GetDeltaTime();

	const float speed = 45.f;
	position += (
		GetForward() * z +
		GetRight() * x +
		GetUp() * y
	) * deltaTime * speed;

	UpdateViewMatrix();
}

glm::vec3 EditorCamera::GetForward() const {
	return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 EditorCamera::GetRight() const {
	return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 EditorCamera::GetUp() const {
	return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

void EditorCamera::ResizeViewport(uint32_t width, uint32_t height) {
	if (this->width == width && this->height == height) {
		return;
	}

	this->width = width;
	this->height = height;

	if (width == 0 || height == 0)
	{
		return;
	}

	GraphicsAPI::Core* core = Editor::Manager::GetEngineCore().GetGraphicsCore();
	core->WaitUntilIdle();

	renderTarget->Resize(width, height);
	depthTarget->Resize(width, height);
	framebuffer->Resize(width, height);
	renderer->Resize(width, height);

	std::pair<GraphicsAPI::Image*, GraphicsAPI::Sampler*> combinedSamplerPair = { renderTarget, sampler };
	GraphicsAPI::DescriptorSet::Binding descriptorSetBinding = GraphicsAPI::DescriptorSet::Binding::CombinedImageSampler( &combinedSamplerPair );

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "Editor Viewport Descriptor Set";
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.bindings = &descriptorSetBinding;
	descriptorSetCreateInfo.layout = descriptorSetLayout;
	descriptorSet = core->CreateDescriptorSet(descriptorSetCreateInfo);

	UpdateProjectionMatrix();
}

void EditorCamera::UpdateProjectionMatrix() {
	float aspectRatio = (float)width / (float)height;
	projection = glm::perspective(fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance);
}

void EditorCamera::UpdateViewMatrix() {
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 target = position + GetForward();
	view = glm::lookAt(position, target, up);
}

glm::mat4& EditorCamera::GetProjectionMatrix() {
	return projection;
}

glm::mat4& EditorCamera::GetViewMatrix() {
	return view;
}

BaseRenderer* EditorCamera::GetRenderer() const {
	return renderer;
}
