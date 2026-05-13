#include <random>

#include <Common/Math.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/ScreenSpaceAmbientOcclusionPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

const size_t ssaoKernelSize = 64;
struct SsaoUboStruct {
	Grindstone::Math::Float4 kernels[ssaoKernelSize];
	float radius;
	float bias;
};

bool Grindstone::Renderer::ScreenSpaceAmbientOcclusionPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;

	ssaoPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/screenSpaceAmbientOcclusion");
	CreateSsaoKernelAndNoise();

	return true;
}

void Grindstone::Renderer::ScreenSpaceAmbientOcclusionPass::CreateSsaoKernelAndNoise() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::uniform_int_distribution<short> randomShorts(0, std::numeric_limits<short>::max());
	std::default_random_engine generator;

	{
		SsaoUboStruct ssaoUboStruct{};
		ssaoUboStruct.bias = 0.025f;
		ssaoUboStruct.radius = 0.1f;

		for (size_t i = 0; i < ssaoKernelSize; ++i)
		{
			glm::vec4 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator),
				0.0f
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = static_cast<float>(i) / static_cast<float>(ssaoKernelSize);
			scale = 0.1f + (0.9f * scale * scale);
			sample *= scale;
			ssaoUboStruct.kernels[i] = sample;
		}

		GraphicsAPI::Buffer::CreateInfo ssaoUniformBufferObjectCi{};
		ssaoUniformBufferObjectCi.debugName = "SSAO Uniform Buffer";
		ssaoUniformBufferObjectCi.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform;
		ssaoUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		ssaoUniformBufferObjectCi.bufferSize = sizeof(SsaoUboStruct);
		ssaoUniformBuffer = graphicsCore->CreateBuffer(ssaoUniformBufferObjectCi);
		ssaoUniformBuffer->UploadData(&ssaoUboStruct);
	}

	{
		constexpr size_t ssaoNoiseDimSize = 4;
		std::array<short, ssaoNoiseDimSize* ssaoNoiseDimSize> ssaoNoise{};
		for (size_t i = 0; i < ssaoNoise.size(); i++) {
			ssaoNoise[i] = randomShorts(generator);
		}

		GraphicsAPI::Image::CreateInfo ssaoNoiseImgCreateInfo{};
		ssaoNoiseImgCreateInfo.debugName = "SSAO Noise Texture";
		ssaoNoiseImgCreateInfo.initialData = reinterpret_cast<const char*>(ssaoNoise.data());
		ssaoNoiseImgCreateInfo.initialDataSize = static_cast<uint32_t>(sizeof(ssaoNoise));
		ssaoNoiseImgCreateInfo.format = GraphicsAPI::Format::R8G8_SNORM;
		ssaoNoiseImgCreateInfo.width = ssaoNoiseImgCreateInfo.height = ssaoNoiseDimSize;
		ssaoNoiseImgCreateInfo.imageUsage =
			GraphicsAPI::ImageUsageFlags::TransferDst |
			GraphicsAPI::ImageUsageFlags::TransferSrc |
			GraphicsAPI::ImageUsageFlags::Sampled;
		ssaoNoiseTexture = graphicsCore->CreateImage(ssaoNoiseImgCreateInfo);
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
		GraphicsAPI::Sampler::CreateInfo ssaoNoiseSamplerCreateInfo{};
		ssaoNoiseSamplerCreateInfo.debugName = "SSAO Noise Sampler";
		ssaoNoiseSamplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Nearest;
		ssaoNoiseSamplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Nearest;
		ssaoNoiseSamplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat;
		ssaoNoiseSamplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat;
		ssaoNoiseSamplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat;
		ssaoNoiseSampler = graphicsCore->GetOrCreateSampler(ssaoNoiseSamplerCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 3> ssaoLayoutBindings{};
		ssaoLayoutBindings[0].bindingId = 0;
		ssaoLayoutBindings[0].count = 1;
		ssaoLayoutBindings[0].type = GraphicsAPI::BindingType::Sampler;
		ssaoLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoLayoutBindings[1].bindingId = 1;
		ssaoLayoutBindings[1].count = 1;
		ssaoLayoutBindings[1].type = GraphicsAPI::BindingType::SampledImage;
		ssaoLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoLayoutBindings[2].bindingId = 2;
		ssaoLayoutBindings[2].count = 1;
		ssaoLayoutBindings[2].type = GraphicsAPI::BindingType::UniformBuffer;
		ssaoLayoutBindings[2].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo ssaoDescriptorSetLayoutCreateInfo{};
		ssaoDescriptorSetLayoutCreateInfo.debugName = "SSAO Input Descriptor Set Layout";
		ssaoDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		ssaoDescriptorSetLayoutCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoInputDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(ssaoDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 3> ssaoLayoutBindings{
			GraphicsAPI::DescriptorSet::Binding::Sampler(ssaoNoiseSampler),
			GraphicsAPI::DescriptorSet::Binding::SampledImage(ssaoNoiseTexture),
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer(ssaoUniformBuffer)
		};

		GraphicsAPI::DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
		engineDescriptorSetCreateInfo.debugName = "SSAO Input Descriptor Set";
		engineDescriptorSetCreateInfo.layout = ssaoInputDescriptorSetLayout;
		engineDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		engineDescriptorSetCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoInputDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);
	}
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::ScreenSpaceAmbientOcclusionPass::AddPass(
	GraphicsAPI::Buffer* vertexBuffer,
	GraphicsAPI::Buffer* indexBuffer,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	const GbufferData& gbufferData
) {
	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"Screenspace Ambient Occlusion Pass",
		Renderer::MetaRect(Renderer::MetaSize2D::Zero(), Renderer::MetaSize2D::DivideSwapchain(2)),
		[this, &gbufferData](
			Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass
		) -> Grindstone::Renderer::RenderGraphBuilderResourceRef {
			Grindstone::Renderer::ImageDescription attachmentAmbientOcclusion{
				.name = "Ambient Occlusion",
				.format = ambientOcclusionFormat,
				.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
			};

			renderPass.ReadExternalSampler(screenSampler);
			renderPass.ReadSampledImage(gbufferData.depthRef);
			renderPass.ReadSampledImage(gbufferData.normalRef);
			RenderGraphBuilderResourceRef ambientOcclusionRef = renderPass.WriteColorAttachment(
				attachmentAmbientOcclusion,
				GraphicsAPI::LoadOp::Clear,
				GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f)
			);

			return ambientOcclusionRef;
		},
		[this, vertexBuffer, indexBuffer](
			Grindstone::Math::IntRect2D viewportArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			GraphicsAPI::CommandBuffer* commandBuffer = cxt.commandBuffer;

			Grindstone::GraphicsPipelineAsset* ssaoPipelineSetAsset = ssaoPipelineSet.Get();
			if (ssaoPipelineSetAsset == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::GraphicsPipeline* ssaoPipeline = ssaoPipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
			if (ssaoPipeline == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::PipelineLayout* ssaoPipelineLayout = ssaoPipeline->pipelineLayout;

			EngineCore& engineCore = EngineCore::GetInstance();
			Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			GraphicsAPI::ClearColor clearColorAttachment = { 16.0f, 16.0f, 16.0f, 16.0f };

			commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
			commandBuffer->BindIndexBuffer(indexBuffer);

			commandBuffer->BindGraphicsPipeline(ssaoPipeline);
			commandBuffer->BindGraphicsDescriptorSet(ssaoPipelineLayout, &ssaoInputDescriptorSet, 2u, 1u);
			commandBuffer->DrawIndices(0, 6, 0, 1, 0);
		}
	);
}
