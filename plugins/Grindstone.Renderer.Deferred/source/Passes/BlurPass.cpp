#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/BlurPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

bool Grindstone::Renderer::BlurPass::Initialize() {
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

	return true;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::BlurPass::AddPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	MetaRect& metaRect,
	Renderer::ImageDescription& imageDescription,
	Renderer::RenderGraphBuilderResourceRef imageToBlurRef
) {
	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"Blur Pass",
		MetaRect::Swapchain(),
		[this, &imageDescription, imageToBlurRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
			renderPass.ReadExternalSampler(screenSampler);
			renderPass.ReadSampledImage(imageToBlurRef);
			// TODO: There should be a way to recover the image description from the reference.
			Renderer::RenderGraphBuilderResourceRef output = renderPass.WriteColorAttachment(imageDescription, GraphicsAPI::LoadOp::DontCare, GraphicsAPI::ClearColor{});

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
				cmd->DrawIndices(0, 6, 0, 1, 0);
		}
	);
}
