#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Features/Tonemap.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <EngineCore/Logger.hpp>

void Grindstone::Renderer::Tonemap::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	tonemapPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/tonemapping");

	GraphicsAPI::Buffer::CreateInfo postProcessingUboCreateInfo{
		.content = nullptr,
		.bufferSize = sizeof(PostProcessSettings),
		.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform,
		.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU,
	};

	GraphicsAPI::DescriptorSetLayout::Binding postProcessingDescriptorSetLayoutBinding{
		.bindingId = 0,
		.count = 1,
		.type = Grindstone::GraphicsAPI::BindingType::UniformBuffer,
		.stages = GraphicsAPI::ShaderStageBit::Fragment,
	};

	GraphicsAPI::DescriptorSetLayout::CreateInfo postProcessingDescriptorSetLayoutCreateInfo{
		.debugName = "Post Processing Settings Descriptor Set Layout",
		.bindings = &postProcessingDescriptorSetLayoutBinding,
		.bindingCount = 1u,
	};

	descriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(postProcessingDescriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::CreateInfo postProcessingDescriptorSetsCreateInfo{
		.layout = descriptorSetLayout,
		.bindingCount = 1u
	};

	for (size_t i = 0; i < 3; ++i) {
		std::string uboDebugName = std::vformat("Post Processing UBO [{}]", std::make_format_args(i));
		postProcessingUboCreateInfo.debugName = uboDebugName.c_str();
		tonemapSettingsUniformBuffer[i] = graphicsCore->CreateBuffer(postProcessingUboCreateInfo);

		std::string descriptorSetDebugName = std::vformat("Post Processing UBO [{}]", std::make_format_args(i));
		GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(tonemapSettingsUniformBuffer[i]);
		postProcessingDescriptorSetsCreateInfo.bindings = &binding;
		postProcessingDescriptorSetsCreateInfo.debugName = descriptorSetDebugName.c_str();
		tonemapSettingsDescriptorSet[i] = graphicsCore->CreateDescriptorSet(postProcessingDescriptorSetsCreateInfo);
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

		screenSampler = engineCore.GetGraphicsCore()->GetOrCreateSampler(screenSamplerCreateInfo);
	}
}

void Grindstone::Renderer::Tonemap::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	PostProcessSettings settings;

	auto lightingImageResponse = context.blackboard.GetValue<Grindstone::Renderer::RenderGraphBuilderResourceRef>("SceneColor");
	if (lightingImageResponse.HasError()) {
		GPRINT_ERROR(LogSource::Rendering, "Tonemap: Unable to get SceneColor: {}", lightingImageResponse.GetError());
		return;
	}
	Grindstone::Renderer::RenderGraphBuilderResourceRef lightingImageRef = lightingImageResponse.GetValue();

	auto bloomImageResponse = context.blackboard.GetValue<Grindstone::Renderer::RenderGraphBuilderResourceRef>("BloomOutput");
	if (bloomImageResponse.HasError()) {
		GPRINT_ERROR(LogSource::Rendering, "Tonemap: Unable to get BloomOutput: {}", bloomImageResponse.GetError());
		return;
	}
	Grindstone::Renderer::RenderGraphBuilderResourceRef bloomImageRef = bloomImageResponse.GetValue();

	Grindstone::Renderer::RenderGraphBuilderResourceRef attachmentOutputRef = renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::TonemapPassReturnData>(
		"Tonemapping",
		MetaRect::Swapchain(),
		[this, lightingImageRef, bloomImageRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::TonemapPassReturnData>& renderPass) {
			renderPass.ReadExternalSampler(screenSampler);
			renderPass.ReadSampledImage(lightingImageRef);
			renderPass.ReadSampledImage(bloomImageRef);
			Renderer::RenderGraphBuilderResourceRef output = renderPass.WriteColorAttachment(attachmentOutputRef, GraphicsAPI::LoadOp::DontCare, GraphicsAPI::ClearColor{});

			return Grindstone::Renderer::TonemapPassReturnData{
				.postProcessOutput = output
			};
		},
		[this, settings](
			Grindstone::Math::IntRect2D renderingArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::TonemapPassReturnData& data
		) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			uint8_t swapchainIndex = cxt.swapchainIndex;

			Grindstone::GraphicsPipelineAsset* tonemapPipelineAsset = tonemapPipelineSet.Get();
			if (tonemapPipelineAsset == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::PipelineLayout* tonemapPipelineLayout = tonemapPipelineAsset->GetFirstPassPipelineLayout();
			Grindstone::GraphicsAPI::GraphicsPipeline* tonemapPipeline = tonemapPipelineAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
			if (tonemapPipeline == nullptr) {
				return;
			}

			tonemapSettingsUniformBuffer[swapchainIndex]->UploadData(&settings);

			cmd->BindGraphicsPipeline(tonemapPipeline);
			cmd->BindGraphicsDescriptorSet(
				tonemapPipelineLayout,
				&tonemapSettingsDescriptorSet[swapchainIndex],
				2u, // Offset
				1u // Count
			);
			cmd->DrawVertices(3, 0, 1, 0);
		}
	);

	context.blackboard.SetValue("Tonemapped", attachmentOutputRef);
}
