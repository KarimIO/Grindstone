#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Features/Blur.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Common/Logger.hpp>

void Grindstone::Renderer::Blur::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	blurPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/screenSpaceAmbientOcclusionBlur");

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

void Grindstone::Renderer::Blur::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	Grindstone::Renderer::ImageDescription attachmentAmbientOcclusion{
		.name = "Ambient Occlusion",
		.format = ambientOcclusionFormat,
		.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
	};

	auto ssaoResponse = context.blackboard.GetValue<Grindstone::Renderer::RenderGraphBuilderResourceRef>("UnblurredAmbientOcclusion");
	if (ssaoResponse.HasError()) {
		GPRINT_ERROR(LogSource::Rendering, "Blur: Unable to get UnblurredAmbientOcclusion: {}", ssaoResponse.GetError());
		return;
	}
	Grindstone::Renderer::RenderGraphBuilderResourceRef ssaoRef = ssaoResponse.GetValue();

	Grindstone::Renderer::RenderGraphBuilderResourceRef blurredRef = renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"Blur Pass",
		MetaRect::Swapchain(),
		[this, &attachmentAmbientOcclusion, ssaoRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
			renderPass.ReadExternalSampler(screenSampler);
			renderPass.ReadSampledImage(ssaoRef);
			// TODO: There should be a way to recover the image description from the reference.
			Renderer::RenderGraphBuilderResourceRef output = renderPass.WriteColorAttachment(attachmentAmbientOcclusion, GraphicsAPI::LoadOp::DontCare, GraphicsAPI::ClearColor{});

			return output;
		},
		[this](
			Grindstone::Math::IntRect2D renderingArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			uint8_t swapchainIndex = cxt.swapchainIndex;

			Grindstone::GraphicsPipelineAsset* blurPipelineSetAsset = blurPipelineSet.Get();
			if (blurPipelineSetAsset == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::PipelineLayout* blurPipelineSetLayout = blurPipelineSetAsset->GetFirstPassPipelineLayout();
			Grindstone::GraphicsAPI::GraphicsPipeline* blurPipelineSet = blurPipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
			if (blurPipelineSet == nullptr) {
				return;
			}

			cmd->BindGraphicsPipeline(blurPipelineSet);
			cmd->DrawVertices(3, 0, 1, 0);
		}
	);

	context.blackboard.SetValue("AmbientOcclusionBlurred", blurredRef);
}
