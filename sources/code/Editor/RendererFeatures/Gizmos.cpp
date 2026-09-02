#include <Common/Console/Cvars.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/RendererFeatures/Gizmos.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/CoreComponents/Lights/PointLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/SpotLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Renderables.3D/include/Components/MeshComponent.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

void Grindstone::Editor::RendererFeatures::Gizmos::Initialize() {
	gizmoRenderer.Initialize();

	Grindstone::GraphicsAPI::Sampler::CreateInfo screenSamplerCreateInfo{
	screenSamplerCreateInfo.debugName = "Screen Sampler",
	screenSamplerCreateInfo.options = {
			.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat,
			.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat,
			.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat,
			.minFilter = GraphicsAPI::TextureFilter::Linear,
			.magFilter = GraphicsAPI::TextureFilter::Linear,
			.anistropy = 0
		}
	};

	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	screenSampler = engineCore.GetGraphicsCore()->GetOrCreateSampler(screenSamplerCreateInfo);
}

void Grindstone::Editor::RendererFeatures::Gizmos::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	Grindstone::Renderer::RenderGraphBuilderResourceRef colorImageRef = context.colorRef;

	for (const Renderer::RenderFrameViewContext& view : context.views) {
		renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
			"Gizmos Pass",
			Grindstone::Renderer::MetaRect::Swapchain(),
			[this, colorImageRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
				// TODO: There should be a way to recover the image description from the reference.
				Renderer::RenderGraphBuilderResourceRef output = renderPass.ReadWriteColorAttachment(colorImageRef);

				return output;
			},
			[this, projView=view.projectionViewMatrix](
				Grindstone::Math::IntRect2D renderingArea,
				const Renderer::RenderGraphContext& cxt,
				const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
				Grindstone::Renderer::RenderGraphBuilderResourceRef& data
			) {
				Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
				Grindstone::Editor::Manager& editorManager = Editor::Manager::GetInstance();
				Grindstone::GraphicsAPI::CommandBuffer* commandBuffer = cxt.commandBuffer;
				Grindstone::GraphicsAPI::Core* graphicsCore = cxt.graphicsCore;

				bool isBoundingSphereGizmoEnabled = true;
				bool isBoundingBoxGizmoEnabled = true;
				bool isColliderGizmoEnabled = true;

				glm::vec2 renderScale = glm::vec2(1.0f, 1.0f);

				if (editorManager.GetSelection().GetSelectedEntityCount() > 0) {
					static const glm::vec4 boundingBoxColor = glm::vec4(0.2f, 0.9f, 0.3f, 1.0f);
					static const glm::vec4 boundingSphereColor = glm::vec4(0.2f, 0.9f, 0.3f, 0.4f);
					static const glm::vec4 colliderColor = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);

					Physics::BoxColliderComponent* box = nullptr;
					Physics::CapsuleColliderComponent* capsule = nullptr;
					Physics::PlaneColliderComponent* plane = nullptr;
					Physics::SphereColliderComponent* sphere = nullptr;
					Grindstone::MeshComponent* mesh = nullptr;
					Grindstone::PointLightComponent* pointLight = nullptr;
					Grindstone::SpotLightComponent* spotLight = nullptr;
					Grindstone::DirectionalLightComponent* directionalLight = nullptr;

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

						if (selectedEntity.TryGetComponent<Grindstone::PointLightComponent>(pointLight)) {
							TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
							Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
							gizmoRenderer.SubmitSphereGizmo(matrix, pointLight->attenuationRadius, glm::vec4(pointLight->color, 1));
						}

						if (selectedEntity.TryGetComponent<Grindstone::SpotLightComponent>(spotLight)) {
							TransformComponent& transf = selectedEntity.GetComponent<TransformComponent>();
							Math::Matrix4 matrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
							gizmoRenderer.SubmitSphereGizmo(matrix, spotLight->attenuationRadius, glm::vec4(spotLight->color, 1));
						}

						if (selectedEntity.TryGetComponent<Grindstone::DirectionalLightComponent>(directionalLight)) {
							static std::array<glm::vec4, DirectionalLightComponent::MAX_CASCADE_COUNT> lightCascadeColors{
								glm::vec4(0, 0, 1, 1),
								glm::vec4(0, 1, 0, 1),
								glm::vec4(0, 1, 1, 1),
								glm::vec4(1, 0, 0, 1),
								glm::vec4(1, 0, 1, 1),
								glm::vec4(1, 1, 0, 1),
								glm::vec4(1, 1, 1, 1),
								glm::vec4(0.5, 0.5, 1, 1),
							};

							float projection_43 = directionalLight->debugCameraProjectionMatrix[3][2];
							float projection_33 = directionalLight->debugCameraProjectionMatrix[2][2];
							float nearDistance = projection_43 / (projection_33 - 1.0f);
							float fov = -2.0 * atan(1.0 / directionalLight->debugCameraProjectionMatrix[1][1]);
							float aspect = -directionalLight->debugCameraProjectionMatrix[1][1] / directionalLight->debugCameraProjectionMatrix[0][0];

							glm::mat4 slice = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.5f));

							Grindstone::CvarSystem* cvarSystem = Grindstone::CvarSystem::GetInstance();
							int32_t csmInspectIndex = *cvarSystem->GetIntCvar(cvarSystem->GetCvar("render.lights.csmInspectIndex"_hash)->arrayIndex);
							for (uint32_t i = 0; i < directionalLight->cascadeCount; ++i) {
								if (i + 1 == csmInspectIndex || csmInspectIndex == 0) {
									Math::Matrix4 shadowCascadeMatrix = glm::inverse(directionalLight->debugShadowProjectionMatrix[i] * directionalLight->shadowViewMatrix[i]);
									gizmoRenderer.SubmitCubeGizmo(shadowCascadeMatrix, glm::vec3(1.0f, 1.0f, 1.0f), lightCascadeColors[i]);

									float nearDist = i == 0 ? nearDistance : directionalLight->cascadeDistances[i - 1];
									float farDist = directionalLight->cascadeDistances[i];

									glm::mat4 cascadeProjView = glm::perspective(fov, aspect, nearDist, farDist);
									Math::Matrix4 cameraCascadeMatrix = glm::inverse(cascadeProjView * directionalLight->debugCameraViewMatrix) * slice;
									gizmoRenderer.SubmitCubeGizmo(cameraCascadeMatrix, glm::vec3(1.0f, 1.0f, 1.0f), lightCascadeColors[i]);
								}
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
			}
		);
	}
}
