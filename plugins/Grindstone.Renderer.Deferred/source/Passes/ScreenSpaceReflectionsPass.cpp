#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/CoreComponents/EnvironmentMap/EnvironmentMapComponent.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ScreenSpaceReflectionsPass.hpp>

Grindstone::Renderer::ImageDescription resourceDesc{
	.name = "Screen-Space Reflections Target",
	.size = Grindstone::Renderer::MetaSize2D::Viewport(),
	.samples = 1,
	.mipLevels = 1,
	.depth = 1,
	.arrayLayers = 1,
	.format = litHdrFormat,
	.imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D,
	.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUOnly,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::Storage | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
};

bool Grindstone::Renderer::ScreenSpaceReflectionsPass::FindEnvironmentMap(
	const entt::registry& registry,
	const Grindstone::AssetReference<Grindstone::TextureAsset> brdfLut,
	Grindstone::GraphicsAPI::DescriptorSet* reflectionDescriptorSet,
	Grindstone::GraphicsAPI::Image*& currentEnvironmentMapImage
) {
	auto view = registry.view<const Grindstone::EnvironmentMapComponent>();

	bool hasEnvMap = false;
	view.each(
		[&hasEnvMap, &currentEnvironmentMapImage, reflectionDescriptorSet](const Grindstone::EnvironmentMapComponent& environmentMapComponent) {
			// Valid env map found - ignore the rest.
			if (hasEnvMap) {
				return;
			}

			const Grindstone::TextureAsset* texAsset = environmentMapComponent.specularTexture.Get();
			// Invalid env map - keep searching.
			if (texAsset == nullptr || texAsset->image == nullptr) {
				return;
			}

			hasEnvMap = true;
			Grindstone::GraphicsAPI::Image* image = texAsset->image;
			if (currentEnvironmentMapImage == image) {
				// We found the correct env map and we're already using it - just send it forward to render.
				return;
			}

			// We found the correct env map and it is different from the current one - change it and then render.
			Grindstone::GraphicsAPI::DescriptorSet::Binding binding = Grindstone::GraphicsAPI::DescriptorSet::Binding::SampledImage(image);
			reflectionDescriptorSet->ChangeBindings(&binding, 1u /* count */, 1u /* offset */);
			currentEnvironmentMapImage = image;
		}
	);

	return hasEnvMap;
}

bool Grindstone::Renderer::ScreenSpaceReflectionsPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	ssrPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/screenSpaceReflections");
	brdfLut = engineCore.assetManager->GetAssetReferenceByAddress<TextureAsset>("@CORESHADERS/textures/ibl_brdf_lut");

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	{
		Grindstone::GraphicsAPI::Sampler::CreateInfo screenSamplerCreateInfo{
			.debugName = "Screen Sampler",
			.options = {
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
				.stages = GraphicsAPI::ShaderStageBit::Compute
			},
			GraphicsAPI::DescriptorSetLayout::Binding{

				.bindingId = 1,
				.count = 1,
				.type = GraphicsAPI::BindingType::SampledImage,
				.stages = GraphicsAPI::ShaderStageBit::Compute
			}
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo ambientOcclusionInputLayoutCreateInfo{};
		ambientOcclusionInputLayoutCreateInfo.debugName = "Reflection Descriptor Set Layout";
		ambientOcclusionInputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ambientOcclusionInputLayoutBinding.size());
		ambientOcclusionInputLayoutCreateInfo.bindings = ambientOcclusionInputLayoutBinding.data();
		reflectionDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ambientOcclusionInputLayoutCreateInfo);
	}

	{
		Grindstone::TextureAsset* brdfLutTextureAsset = brdfLut.Get();
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> aoInputBinding = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage(brdfLutTextureAsset != nullptr ? brdfLutTextureAsset->image : nullptr),
			GraphicsAPI::DescriptorSet::Binding::SampledImage(nullptr)
		};

		GraphicsAPI::DescriptorSet::CreateInfo aoInputCreateInfo{};
		aoInputCreateInfo.debugName = "Reflection Descriptor Set";
		aoInputCreateInfo.layout = reflectionDescriptorSetLayout;
		aoInputCreateInfo.bindingCount = static_cast<uint32_t>(aoInputBinding.size());
		aoInputCreateInfo.bindings = aoInputBinding.data();
		reflectionDescriptorSet = graphicsCore->CreateDescriptorSet(aoInputCreateInfo);
	}
	
	return true;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::ScreenSpaceReflectionsPass::AddPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Renderer::RenderGraphBuilderResourceRef inputRef,
	Renderer::RenderGraphBuilderResourceRef ambientOcclusionRef,
	Grindstone::Renderer::GbufferData& gbufferData
) {
	Grindstone::ComputePipelineAsset* ssrPipelineAsset = ssrPipelineSet.Get();
	if (ssrPipelineAsset == nullptr) {
		return inputRef;
	}

	Grindstone::GraphicsAPI::ComputePipeline* ssrPipeline = ssrPipelineAsset->GetPipeline();
	Grindstone::GraphicsAPI::PipelineLayout* ssrPipelineLayout = ssrPipelineAsset->GetPipelineLayout();
	if (ssrPipeline == nullptr || ssrPipelineLayout == nullptr) {
		return inputRef;
	}

	return renderGraphBuilder.CreateComputePass<RenderGraphBuilderResourceRef>(
		"Screen-Space Reflections",
		[this, inputRef, &gbufferData, ambientOcclusionRef](ComputeRenderGraphBuilderPass<RenderGraphBuilderResourceRef>& pass) -> RenderGraphBuilderResourceRef {
			pass.ReadExternalSampler(screenSampler);
			auto outputRef = pass.WriteStorageImage(resourceDesc);
			pass.ReadSampledImage(inputRef);
			pass.ReadSampledImage(gbufferData.depthRef);
			pass.ReadSampledImage(gbufferData.normalRef);
			pass.ReadSampledImage(gbufferData.specularRoughnessRef);
			pass.ReadSampledImage(ambientOcclusionRef);

			return outputRef;
		},
		[ssrPipeline, ssrPipelineLayout, this](
			RenderGraphContext& cxt,
			const RenderGraphFrameResources& frameResources,
			RenderGraphBuilderResourceRef& ref
		) {
			FindEnvironmentMap(cxt.worldContextSet->GetEntityRegistry(), brdfLut, reflectionDescriptorSet, currentEnvironmentMapImage);
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

			constexpr uint32_t WORKGROUP_SIZE = 4;
			uint32_t groupCountX = (cxt.swapchainSize.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
			uint32_t groupCountY = (cxt.swapchainSize.y + WORKGROUP_SIZE - 1 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

			cmd->BindComputePipeline(ssrPipeline);

			cmd->BindComputeDescriptorSet(
				ssrPipelineLayout,
				&reflectionDescriptorSet,
				2u, // Offset
				1u // Count
			);

			cmd->DispatchCompute(groupCountX, groupCountY, 1u);
		}
	);
}
