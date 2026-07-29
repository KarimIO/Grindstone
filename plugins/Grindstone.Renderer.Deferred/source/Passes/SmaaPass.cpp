#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/SmaaPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

const Grindstone::ConstHashedString smaaRenderPassHashedString("smaa_rg");

Grindstone::Renderer::ImageDescription smaaEdgesAttachmentDesc{
	.name = "SMAA Edges",
	.format = Grindstone::GraphicsAPI::Format::R8G8_UNORM,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
};

Grindstone::Renderer::ImageDescription smaaBlendAttachmentDesc{
	.name = "SMAA Blend",
	.format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UNORM,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
};

static Grindstone::Renderer::RenderGraphBuilderResourceRef SmaaEdgeDetectionPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::GraphicsPipelineAsset* smaaPipelineAsset,
	Grindstone::Renderer::RenderGraphBuilderResourceRef lightingImageRef,
	Grindstone::GraphicsAPI::Sampler* pointSampler
) {
	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"SMAA - EdgeDetection",
		Grindstone::Renderer::MetaRect::Swapchain(),
		[pointSampler, lightingImageRef](Grindstone::Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
			renderPass.ReadExternalSampler(pointSampler);
			renderPass.ReadSampledImage(lightingImageRef);
			Grindstone::Renderer::RenderGraphBuilderResourceRef output = renderPass.WriteColorAttachment(smaaEdgesAttachmentDesc, Grindstone::GraphicsAPI::LoadOp::Clear, Grindstone::GraphicsAPI::ClearColor{0.0f, 0.0f, 0.0f, 0.0f});
			return output;
		},
		[smaaPipelineAsset](
			Grindstone::Math::IntRect2D renderingArea,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

			Grindstone::GraphicsAPI::GraphicsPipeline* smaaPipeline = smaaPipelineAsset->GetPassPipelineByName("EdgeDetection", &vertexLightPositionLayout);
			if (smaaPipeline == nullptr) {
				return;
			}

			cmd->BindGraphicsPipeline(smaaPipeline);
			cmd->DrawIndices(0, 6, 0, 1, 0);
		}
	);
}

static Grindstone::Renderer::RenderGraphBuilderResourceRef SmaaBlendWeightCalculationPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::GraphicsAPI::DescriptorSet* smaaDescriptorSet,
	Grindstone::GraphicsPipelineAsset* smaaPipelineAsset,
	Grindstone::Renderer::RenderGraphBuilderResourceRef edgesImageRef,
	Grindstone::GraphicsAPI::Sampler* linearSampler
) {
	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"SMAA - BlendingWeightCalculation",
		Grindstone::Renderer::MetaRect::Swapchain(),
		[edgesImageRef, linearSampler](Grindstone::Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
			renderPass.ReadExternalSampler(linearSampler);
			renderPass.ReadSampledImage(edgesImageRef);
			Grindstone::Renderer::RenderGraphBuilderResourceRef output = renderPass.WriteColorAttachment(smaaBlendAttachmentDesc, Grindstone::GraphicsAPI::LoadOp::Clear, Grindstone::GraphicsAPI::ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f });
			return output;
		},
		[smaaPipelineAsset, smaaDescriptorSet](
			Grindstone::Math::IntRect2D renderingArea,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

			const char* pipelinePassName = "BlendingWeight";
			const Grindstone::GraphicsPipelineAsset::Pass* smaaPipelinePass = smaaPipelineAsset->GetPassByName(pipelinePassName);
			if (smaaPipelinePass == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::PipelineLayout* smaaPipelineLayout = smaaPipelinePass->pipelineLayout;
			Grindstone::GraphicsAPI::GraphicsPipeline* smaaPipeline = smaaPipelineAsset->GetPassPipelineByName(pipelinePassName, &vertexLightPositionLayout);
			if (smaaPipelineLayout == nullptr) {
				return;
			}

			cmd->BindGraphicsPipeline(smaaPipeline);
			cmd->BindGraphicsDescriptorSet(
				smaaPipelineLayout,
				&smaaDescriptorSet,
				2u, // Offset
				1u // Count
			);
			cmd->DrawIndices(0, 6, 0, 1, 0);
		}
	);
}

static Grindstone::Renderer::RenderGraphBuilderResourceRef SmaaNeighborhoodBlendingPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::GraphicsPipelineAsset* smaaPipelineAsset,
	Grindstone::Renderer::RenderGraphBuilderResourceRef litImageImageRef,
	Grindstone::Renderer::RenderGraphBuilderResourceRef blendImageRef,
	Grindstone::Renderer::RenderGraphBuilderResourceRef outputImageRef,
	Grindstone::GraphicsAPI::Sampler* linearSampler
) {
	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"SMAA - NeighborhoodBlending",
		Grindstone::Renderer::MetaRect::Swapchain(),
		[litImageImageRef, blendImageRef, outputImageRef, linearSampler](Grindstone::Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
			renderPass.ReadExternalSampler(linearSampler);
			renderPass.ReadSampledImage(litImageImageRef);
			renderPass.ReadSampledImage(blendImageRef);
			Grindstone::Renderer::RenderGraphBuilderResourceRef output = renderPass.WriteColorAttachment(outputImageRef, Grindstone::GraphicsAPI::LoadOp::Clear, Grindstone::GraphicsAPI::ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f });
			return output;
		},
		[smaaPipelineAsset](
			Grindstone::Math::IntRect2D renderingArea,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

			Grindstone::GraphicsAPI::GraphicsPipeline* smaaPipeline = smaaPipelineAsset->GetPassPipelineByName("NeighborhoodBlending", &vertexLightPositionLayout);
			if (smaaPipeline == nullptr) {
				return;
			}

			cmd->BindGraphicsPipeline(smaaPipeline);
			cmd->DrawIndices(0, 6, 0, 1, 0);
		}
	);
}

bool Grindstone::Renderer::SmaaPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::RenderPassRegistry* renderPassRegistry = engineCore.GetRenderPassRegistry();

	{
		GraphicsAPI::RenderPass::AttachmentInfo attachment{ GraphicsAPI::Format::R8G8_UNORM, true };
		GraphicsAPI::RenderPass::CreateInfo editorRenderPassCreateInfo{};
		editorRenderPassCreateInfo.debugName = "SMAA_EdgeDetection";
		editorRenderPassCreateInfo.colorAttachmentCount = 1u;
		editorRenderPassCreateInfo.colorAttachments = &attachment;
		editorRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::Invalid;
		editorRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		Grindstone::GraphicsAPI::RenderPass* smaaRenderPass = graphicsCore->CreateRenderPass(editorRenderPassCreateInfo);
		renderPassRegistry->RegisterRenderpass(smaaRenderPassHashedString, smaaRenderPass);
	}

	smaaPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/smaa");
	smaaAreaTexture = engineCore.assetManager->GetAssetReferenceByAddress<TextureAsset>("@CORESHADERS/textures/smaa/area");
	smaaSearchTexture = engineCore.assetManager->GetAssetReferenceByAddress<TextureAsset>("@CORESHADERS/textures/smaa/search");

	if (!smaaPipelineSet.IsValid() ||
		!smaaAreaTexture.IsValid() ||
		!smaaSearchTexture.IsValid() ||
		smaaAreaTexture.Get()->image == nullptr ||
		smaaSearchTexture.Get()->image == nullptr
	) {
		GS_ASSERT(false && "Unable to get area or search texture.");
		return false;
	}

	TextureAsset* areaTex = smaaAreaTexture.Get();
	TextureAsset* searchTex = smaaSearchTexture.Get();

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> layoutBindings{
			GraphicsAPI::DescriptorSetLayout::Binding{0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment},
			GraphicsAPI::DescriptorSetLayout::Binding{1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment},
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo smaaDescriptorSetLayoutCreateInfo{
			.debugName = "SMAA Blending Descriptor Set Layout",
			.bindings = layoutBindings.data(),
			.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
		};

		smaaDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(smaaDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> bindings{
			GraphicsAPI::DescriptorSet::Binding::SampledImage(areaTex->image),
			GraphicsAPI::DescriptorSet::Binding::SampledImage(searchTex->image),
		};

		GraphicsAPI::DescriptorSet::CreateInfo smaaDescriptorSetsCreateInfo{
			.debugName = "SMAA Blending Descriptor Set",
			.layout = smaaDescriptorSetLayout,
			.bindings = bindings.data(),
			.bindingCount = static_cast<uint32_t>(bindings.size()),
		};
		smaaDescriptorSet = graphicsCore->CreateDescriptorSet(smaaDescriptorSetsCreateInfo);
	}

	{
		Grindstone::GraphicsAPI::Sampler::CreateInfo pointSamplerCreateInfo{
			.debugName = "Point Sampler",
			.options = {
				.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat,
				.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat,
				.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat,
				.minFilter = GraphicsAPI::TextureFilter::Nearest,
				.magFilter = GraphicsAPI::TextureFilter::Nearest,
				.anistropy = 0
			}
		};
		pointSampler = graphicsCore->GetOrCreateSampler(pointSamplerCreateInfo);
	}

	{
		Grindstone::GraphicsAPI::Sampler::CreateInfo linearSamplerCreateInfo{
			.debugName = "Linear Sampler",
			.options = {
				.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat,
				.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat,
				.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat,
				.minFilter = GraphicsAPI::TextureFilter::Linear,
				.magFilter = GraphicsAPI::TextureFilter::Linear,
				.anistropy = 0
			}
		};
		linearSampler = graphicsCore->GetOrCreateSampler(linearSamplerCreateInfo);
	}

	return true;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::SmaaPass::AddPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Renderer::RenderGraphBuilderResourceRef litImageRef,
	Renderer::RenderGraphBuilderResourceRef outputImageRef
) {
	Grindstone::GraphicsPipelineAsset* smaaPipelineAsset = smaaPipelineSet.Get();
	if (false && smaaPipelineAsset == nullptr) {
		return litImageRef;
	}

	Grindstone::Renderer::RenderGraphBuilderResourceRef edgesImageRef = SmaaEdgeDetectionPass(
		renderGraphBuilder,
		smaaPipelineAsset,
		litImageRef,
		pointSampler
	);

	Grindstone::Renderer::RenderGraphBuilderResourceRef blendImageRef = SmaaBlendWeightCalculationPass(
		renderGraphBuilder,
		smaaDescriptorSet,
		smaaPipelineAsset,
		edgesImageRef,
		linearSampler
	);

	Grindstone::Renderer::RenderGraphBuilderResourceRef finalAAImageRef = SmaaNeighborhoodBlendingPass(
		renderGraphBuilder,
		smaaPipelineAsset,
		litImageRef,
		blendImageRef,
		outputImageRef, // TODO: This is passed in as an external img but may be better to just use returned ref.
		linearSampler
	);

	return litImageRef;
}
