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
		GraphicsAPI::PipelineLayout* imageBasedLightingPipelineLayout = imageBasedLightingAsset->GetFirstPassPipelineLayout();
		GraphicsAPI::GraphicsPipeline* imageBasedLightingPipeline = imageBasedLightingAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (imageBasedLightingPipeline != nullptr) {
			cmd->BeginDebugLabelSection("Image Based Lighting", nullptr);
			cmd->BindGraphicsPipeline(imageBasedLightingPipeline);

			auto view = registry.view<const EnvironmentMapComponent>();

			bool hasEnvMap = false;
			view.each(
				[&hasEnvMap, &currentEnvironmentMapImage, &ambientOcclusionDescriptorSet](const EnvironmentMapComponent& environmentMapComponent) {
					// Valid env map found - ignore the rest.
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
					ambientOcclusionDescriptorSet->ChangeBindings(&binding, 1u /* count */, 1u /* offset */);
					currentEnvironmentMapImage = image;
				}
			);

			if (hasEnvMap) {
				cmd->BindGraphicsDescriptorSet(
					imageBasedLightingPipelineLayout,
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
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore.assetManager;

	{
		imageBasedLightingPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/ibl");
		pointLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/point");
		spotLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/spot");
		directionalLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/directional");
		brdfLut = assetManager->GetAssetReferenceByAddress<TextureAsset>("@CORESHADERS/textures/ibl_brdf_lut");
	}

	{
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

		screenSampler = graphicsCore->GetOrCreateSampler(screenSamplerCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> ambientOcclusionInputLayoutBinding{
			GraphicsAPI::DescriptorSetLayout::Binding{
				.bindingId = 0,
				.count = 1,
				.type = GraphicsAPI::BindingType::SampledImage,
				.stages = GraphicsAPI::ShaderStageBit::Fragment
			},
			GraphicsAPI::DescriptorSetLayout::Binding{

				.bindingId = 1,
				.count = 1,
				.type = GraphicsAPI::BindingType::SampledImage,
				.stages = GraphicsAPI::ShaderStageBit::Fragment
			}
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo ambientOcclusionInputLayoutCreateInfo{};
		ambientOcclusionInputLayoutCreateInfo.debugName = "Ambient Occlusion Descriptor Set Layout";
		ambientOcclusionInputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ambientOcclusionInputLayoutBinding.size());
		ambientOcclusionInputLayoutCreateInfo.bindings = ambientOcclusionInputLayoutBinding.data();
		ambientOcclusionDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ambientOcclusionInputLayoutCreateInfo);
	}

	{
		Grindstone::TextureAsset* brdfLutTextureAsset = brdfLut.Get();
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> aoInputBinding = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage(brdfLutTextureAsset != nullptr ? brdfLutTextureAsset->image : nullptr),
			GraphicsAPI::DescriptorSet::Binding::SampledImage(nullptr)
		};

		GraphicsAPI::DescriptorSet::CreateInfo aoInputCreateInfo{};
		aoInputCreateInfo.debugName = "Ambient Occlusion Descriptor Set";
		aoInputCreateInfo.layout = ambientOcclusionDescriptorSetLayout;
		aoInputCreateInfo.bindingCount = static_cast<uint32_t>(aoInputBinding.size());
		aoInputCreateInfo.bindings = aoInputBinding.data();
		ambientOcclusionDescriptorSet = graphicsCore->CreateDescriptorSet(aoInputCreateInfo);
	}

	return true;
}

Grindstone::Renderer::LightingPassReturnData Renderer::LightingPass::AddPass(
	GraphicsAPI::Buffer* vertexBuffer,
	GraphicsAPI::Buffer* indexBuffer,
	Renderer::RenderGraphBuilder& renderGraph,
	Grindstone::Renderer::GbufferData& gbufferData,
	RenderGraphBuilderResourceRef shadowAtlasRef,
	RenderGraphBuilderResourceRef ambientOcclusionRef
) {
	return renderGraph.CreateGraphicsPass<LightingPassReturnData>(
		"Lighting Pass",
		MetaRect::Swapchain(),
		[this, shadowAtlasRef, ambientOcclusionRef, &gbufferData](Renderer::GraphicsRenderGraphBuilderPass<LightingPassReturnData>& renderPass) -> LightingPassReturnData {
			renderPass.ReadExternalSampler(screenSampler);
			renderPass.ReadSampledImage(gbufferData.depthRef);
			renderPass.ReadSampledImage(gbufferData.albedoRef);
			renderPass.ReadSampledImage(gbufferData.normalRef);
			renderPass.ReadSampledImage(gbufferData.specularRoughnessRef);
			renderPass.ReadSampledImage(shadowAtlasRef);
			renderPass.ReadSampledImage(ambientOcclusionRef);
			RenderGraphBuilderResourceRef layoutImgRef = renderPass.WriteColorAttachment(attachmentlighting, GraphicsAPI::LoadOp::Clear, GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 1.0f));

			return LightingPassReturnData{
				.lightingOutputRef = layoutImgRef
			};
		},
		[vertexBuffer, indexBuffer, this](
			Grindstone::Math::IntRect2D viewportArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			LightingPassReturnData& lightingImageRef
		) {
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

			GraphicsPipelineAsset* imageBasedLightingAsset = imageBasedLightingPipelineSet.Get();
			if (imageBasedLightingAsset != nullptr) {
				PerformImageBasedLighting(
					registry,
					cmd,
					ambientOcclusionDescriptorSet,
					imageBasedLightingPipelineSet.Get(),
					currentEnvironmentMapImage
				);
			}

			GraphicsPipelineAsset* pointLightAsset = pointLightPipelineSet.Get();
			if (pointLightAsset != nullptr) {
				GraphicsAPI::PipelineLayout* pointLightPipelineLayout = pointLightAsset->GetFirstPassPipelineLayout();
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
								pointLightPipelineLayout,
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
				GraphicsAPI::PipelineLayout* spotLightPipelineLayout = spotLightAsset->GetFirstPassPipelineLayout();
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
								spotLightComponent.shadowRenderArea
							};
							spotLightComponent.uniformBufferObject->UploadData(&lightStruct);

							cmd->BindGraphicsDescriptorSet(
								spotLightPipelineLayout,
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
				GraphicsAPI::PipelineLayout* directionalLightPipelineLayout = directionalLightAsset->GetFirstPassPipelineLayout();
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
								directionalLightComponent.shadowRenderArea
							};

							directionalLightComponent.uniformBufferObject->UploadData(&lightStruct);

							cmd->BindGraphicsDescriptorSet(
								directionalLightPipelineLayout,
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
