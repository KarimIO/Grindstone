#include <random>

#include <Common/Math.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Debug.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Gbuffer.hpp>

struct DebugUboData {
	uint32_t mode;
	float nearDistance;
	float farDistance;
};

void Grindstone::Renderer::Debug::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	debugPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/editor/debug");

	GraphicsAPI::Buffer::CreateInfo postProcessingUboCreateInfo{
		.content = nullptr,
		.bufferSize = sizeof(DebugUboData),
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

	GraphicsAPI::DescriptorSetLayout::CreateInfo debugDataDescriptorSetLayoutCreateInfo{
		.debugName = "Debug Data Descriptor Set Layout",
		.bindings = &postProcessingDescriptorSetLayoutBinding,
		.bindingCount = 1u,
	};

	descriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(debugDataDescriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::CreateInfo debugDescriptorSetsCreateInfo{
		.layout = descriptorSetLayout,
		.bindingCount = 1u
	};

	for (size_t i = 0; i < 3; ++i) {
		std::string uboDebugName = std::vformat("Debug UBO [{}]", std::make_format_args(i));
		postProcessingUboCreateInfo.debugName = uboDebugName.c_str();
		debugDataUniformBuffer[i] = graphicsCore->CreateBuffer(postProcessingUboCreateInfo);

		std::string descriptorSetDebugName = std::vformat("Debug UBO Descriptor Set [{}]", std::make_format_args(i));
		GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(debugDataUniformBuffer[i]);
		debugDescriptorSetsCreateInfo.bindings = &binding;
		debugDescriptorSetsCreateInfo.debugName = descriptorSetDebugName.c_str();
		debugDataDescriptorSet[i] = graphicsCore->CreateDescriptorSet(debugDescriptorSetsCreateInfo);
	}

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

void Grindstone::Renderer::Debug::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	auto ambientOcclusionResponse = context.blackboard.GetValue<Grindstone::Renderer::RenderGraphBuilderResourceRef>("AmbientOcclusionBlurred");
	if (ambientOcclusionResponse.HasError()) {
		GPRINT_ERROR_V(LogSource::Rendering, "ScreenSpaceReflections: Unable to get blurred SSAO: {}", ambientOcclusionResponse.GetError());
		return;
	}
	Grindstone::Renderer::RenderGraphBuilderResourceRef ambientOcclusionRef = ambientOcclusionResponse.GetValue();

	auto gbufferDataResponse = context.blackboard.GetValue<Grindstone::Renderer::GbufferData>();
	if (gbufferDataResponse.HasError()) {
		GPRINT_ERROR_V(LogSource::Rendering, "ScreenSpaceReflections: Unable to get Gbuffer: {}", gbufferDataResponse.GetError());
		return;
	}
	Grindstone::Renderer::GbufferData gbufferData = gbufferDataResponse.GetValue();

	RenderGraphBuilderResourceRef outputRef = context.colorRef;

	// TODO: RenderGraph 2.0 Handle this.
	DeferredRenderMode renderMode = DeferredRenderMode::Default;

	for(const RenderFrameViewContext& view : context.views) {
		const glm::mat4& projectionMatrix = view.projectionMatrix;

		renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
			"Debug Pass",
			Renderer::MetaRect::Swapchain(),
			[this, &gbufferData, ambientOcclusionRef, outputRef](
				Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass
			) -> Grindstone::Renderer::RenderGraphBuilderResourceRef {
				renderPass.ReadExternalSampler(screenSampler);
				renderPass.ReadSampledImage(gbufferData.depthRef);
				renderPass.ReadSampledImage(gbufferData.albedoRef);
				renderPass.ReadSampledImage(gbufferData.normalRef);
				renderPass.ReadSampledImage(gbufferData.specularRoughnessRef);
				renderPass.ReadSampledImage(ambientOcclusionRef);
				Grindstone::Renderer::RenderGraphBuilderResourceRef debugOutputRef = renderPass.WriteColorAttachment(outputRef, Grindstone::GraphicsAPI::LoadOp::DontCare, Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 1.0f));

				return debugOutputRef;
			},
			[this, &projectionMatrix, renderMode](
				Grindstone::Math::IntRect2D viewportArea,
				const Renderer::RenderGraphContext& cxt,
				const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
				Grindstone::Renderer::RenderGraphBuilderResourceRef& data
			) {
				GraphicsAPI::CommandBuffer* commandBuffer = cxt.commandBuffer;
				uint32_t swapchainIndex = cxt.swapchainIndex;

				Grindstone::GraphicsPipelineAsset* debugPipelineSetAsset = debugPipelineSet.Get();
				if (debugPipelineSetAsset == nullptr) {
					return;
				}

				Grindstone::GraphicsAPI::PipelineLayout* debugPipelineLayout = debugPipelineSetAsset->GetFirstPassPipelineLayout();
				Grindstone::GraphicsAPI::GraphicsPipeline* debugPipeline = debugPipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
				if (debugPipeline == nullptr) {
					return;
				}

				float projection_43 = projectionMatrix[3][2];
				float projection_33 = projectionMatrix[2][2];

				DebugUboData debugData{
					.mode = static_cast<uint32_t>(renderMode),
					.nearDistance = projection_43 / (projection_33 - 1.0f),
					.farDistance = projection_43 / (projection_33 + 1.0f)
				};

				debugDataUniformBuffer[swapchainIndex]->UploadData(&debugData);

				commandBuffer->BindGraphicsDescriptorSet(
					debugPipelineLayout,
					&debugDataDescriptorSet[swapchainIndex],
					2u, // Offset
					1u // Count
				);

				commandBuffer->BindGraphicsPipeline(debugPipeline);
				commandBuffer->DrawVertices(3, 0, 1, 0);
			}
		);
	}
}
