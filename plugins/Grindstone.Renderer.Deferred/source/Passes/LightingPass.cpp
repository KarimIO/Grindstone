#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Scenes/Manager.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/LightingPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/CoreComponents/EnvironmentMap/EnvironmentMapComponent.hpp>
#include <EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/PointLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/SpotLightComponent.hpp>

using namespace Grindstone;

static void PerformImageBasedLighting(
	entt::registry& registry,
	GraphicsAPI::CommandBuffer* cmd,
	GraphicsAPI::DescriptorSet* ambientOcclusionDescriptorSet,
	GraphicsPipelineAsset* imageBasedLightingAsset,
	Grindstone::GraphicsAPI::Image*& currentEnvironmentMapImage
) {
	if (imageBasedLightingAsset != nullptr) {
		GraphicsAPI::GraphicsPipeline* imageBasedLightingPipeline = imageBasedLightingAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (imageBasedLightingPipeline != nullptr) {
			cmd->BeginDebugLabelSection("Image Based Lighting", nullptr);
			cmd->BindGraphicsPipeline(imageBasedLightingPipeline);

			auto view = registry.view<const EnvironmentMapComponent>();

			bool hasEnvMap = false;
			view.each(
				[&hasEnvMap, &currentEnvironmentMapImage, &ambientOcclusionDescriptorSet](const EnvironmentMapComponent& environmentMapComponent) {
					// Valid env map found - ignore ther est.
					if (hasEnvMap) {
						return;
					}

					const TextureAsset* texAsset = environmentMapComponent.specularTexture.Get();
					// Invalid env map - keep searching.
					if (texAsset == nullptr || texAsset->image == nullptr) {
						return;
					}

					hasEnvMap = true;
					GraphicsAPI::Image* image = texAsset->image;
					if (currentEnvironmentMapImage == image) {
						// We found the correct env map and we're already using it - just send it forward to render.
						return;
					}

					// We found the correct env map and it is different from the current one - change it and then render.
					GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::SampledImage(image);
					ambientOcclusionDescriptorSet->ChangeBindings(&binding, 1, 2);
					currentEnvironmentMapImage = image;
				}
			);

			if (hasEnvMap) {
				cmd->BindGraphicsDescriptorSet(
					imageBasedLightingPipeline,
					&ambientOcclusionDescriptorSet,
					2u, // Offset
					1u // Count
				);
				cmd->DrawIndices(0, 6, 0, 1, 0);
			}

			cmd->EndDebugLabelSection();
		}
	}
}

bool Renderer::LightingPass::Initialize() {
	EngineCore& engineCore = EngineCore::GetInstance();
	Assets::AssetManager* assetManager = engineCore.assetManager;

	imageBasedLightingPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/ibl");
	pointLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/point");
	spotLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/spot");
	directionalLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/directional");

	return true;
}

void Renderer::LightingPass::AddPass(GraphicsAPI::Buffer* vertexBuffer, GraphicsAPI::Buffer* indexBuffer, Renderer::RenderGraph& renderGraph) {
	renderGraph.AddGraphicsPass(
		"Lighting Pass"_hash,
		[&](Renderer::RenderGraph::RenderPass& renderPass) {
			renderPass.ReadColorAttachment(attachmentNameAlbedo, attachmentAlbedo);
			renderPass.ReadColorAttachment(attachmentNameNormal, attachmentNormal);
			renderPass.ReadColorAttachment(attachmentNameSpecularRoughness, attachmentSpecularRoughness);
			renderPass.ReadDepthStencilAttachment(attachmentNameDepthStencil, attachmentDepthStencil);
			renderPass.WriteColorAttachment(attachmentNameLighting, attachmentlighting, GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		},
		[vertexBuffer, indexBuffer, this](Renderer::RenderGraph::RenderGraphContext& cxt, Renderer::RenderGraph::RenderPassExecution& renderPassExecution) {
			GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			EngineCore& engineCore = EngineCore::GetInstance();
			entt::registry& registry = cxt.worldContextSet->GetEntityRegistry();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			// TODO: We should be able to get transforms below without a scene.
			Grindstone::SceneManagement::SceneManager* sceneManager = engineCore.GetSceneManager();
			Grindstone::SceneManagement::Scene* scene = sceneManager->scenes.begin()->second;

			cmd->BindVertexBuffers(&vertexBuffer, 1);
			cmd->BindIndexBuffer(indexBuffer);


			const glm::mat4 bias = glm::mat4(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.0f, 1.0f
			);

			GraphicsPipelineAsset* pointLightAsset = pointLightPipelineSet.Get();
			if (pointLightAsset != nullptr) {
				GraphicsAPI::GraphicsPipeline* pointLightPipeline = pointLightAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
				if (pointLightPipeline != nullptr) {
					cmd->BeginDebugLabelSection("Point Lighting", nullptr);
					cmd->BindGraphicsPipeline(pointLightPipeline);

					auto view = registry.view<const entt::entity, PointLightComponent>();
					view.each(
						[&](const entt::entity entityHandle, PointLightComponent& pointLightComponent) {
							const ECS::Entity entity(entityHandle, scene);
							PointLightComponent::UniformStruct lightmapStruct{
								pointLightComponent.color,
								pointLightComponent.attenuationRadius,
								entity.GetWorldPosition(),
								pointLightComponent.intensity
							};

							pointLightComponent.uniformBufferObject->UploadData(&lightmapStruct);
							cmd->BindGraphicsDescriptorSet(
								pointLightPipeline,
								&pointLightComponent.descriptorSet,
								2u, // Offset
								1u // Count
							);
							cmd->DrawIndices(0, 6, 0, 1, 0);
						}
					);
					cmd->EndDebugLabelSection();
				}
			}

			GraphicsPipelineAsset* spotLightAsset = spotLightPipelineSet.Get();
			if (spotLightAsset != nullptr) {
				GraphicsAPI::GraphicsPipeline* spotLightPipeline = spotLightAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
				if (spotLightPipeline != nullptr) {
					cmd->BeginDebugLabelSection("Spot Lighting", nullptr);
					cmd->BindGraphicsPipeline(spotLightPipeline);

					auto view = registry.view<const entt::entity, SpotLightComponent>();
					view.each(
						[&](const entt::entity entityHandle, SpotLightComponent& spotLightComponent) {
							const ECS::Entity entity(entityHandle, scene);

							SpotLightComponent::UniformStruct lightStruct{
								bias * spotLightComponent.shadowMatrix,
								spotLightComponent.color,
								spotLightComponent.attenuationRadius,
								entity.GetWorldPosition(),
								spotLightComponent.intensity,
								entity.GetWorldForward(),
								glm::cos(glm::radians(spotLightComponent.innerAngle)),
								glm::cos(glm::radians(spotLightComponent.outerAngle)),
								static_cast<float>(spotLightComponent.shadowResolution)
							};

							spotLightComponent.uniformBufferObject->UploadData(&lightStruct);

							cmd->BindGraphicsDescriptorSet(
								spotLightPipeline,
								&spotLightComponent.descriptorSet,
								2u, // Offset
								1u // Count
							);
							cmd->DrawIndices(0, 6, 0, 1, 0);
						}
					);
					cmd->EndDebugLabelSection();
				}
			}

			GraphicsPipelineAsset* directionalLightAsset = directionalLightPipelineSet.Get();
			if (directionalLightAsset != nullptr) {
				GraphicsAPI::GraphicsPipeline* directionalLightPipeline = directionalLightAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
				if (directionalLightPipeline != nullptr) {
					cmd->BeginDebugLabelSection("Directional Lighting", nullptr);
					cmd->BindGraphicsPipeline(directionalLightPipeline);

					auto view = registry.view<const entt::entity, const TransformComponent, DirectionalLightComponent>();
					view.each(
						[&](const entt::entity entityHandle, const TransformComponent& transformComponent, DirectionalLightComponent& directionalLightComponent) {
							const ECS::Entity entity(entityHandle, scene);

							DirectionalLightComponent::UniformStruct lightStruct{
								bias * directionalLightComponent.shadowMatrix,
								directionalLightComponent.color,
								directionalLightComponent.sourceRadius,
								entity.GetWorldForward(),
								directionalLightComponent.intensity,
								static_cast<float>(directionalLightComponent.shadowResolution)
							};

							directionalLightComponent.uniformBufferObject->UploadData(&lightStruct);

							cmd->BindGraphicsDescriptorSet(
								directionalLightPipeline,
								&directionalLightComponent.descriptorSet,
								2u, // Offset
								1u // Count
							);
							cmd->DrawIndices(0, 6, 0, 1, 0);
						}
					);
					cmd->EndDebugLabelSection();
				}
			}
		}
	);
}
