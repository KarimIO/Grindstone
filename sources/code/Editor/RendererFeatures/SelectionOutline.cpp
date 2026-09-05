#include <Common/Graphics/Buffer.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Gbuffer.hpp>
#include <Editor/RendererFeatures/SelectionOutline.hpp>

static Grindstone::Renderer::ImageDescription resource{
	.name = "Selection Outline Color Attachment",
	.size = Grindstone::Renderer::MetaSize2D::Viewport(),
	.samples = 1,
	.mipLevels = 1,
	.depth = 1,
	.arrayLayers = 1,
	.format = Grindstone::GraphicsAPI::Format::R32_UINT,
	.imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D,
	.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUOnly,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::Sampled | Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget
};

static Grindstone::Renderer::ImageDescription depthResourceDesc{
	.name = "Selection Outline Depth Attachment",
	.size = Grindstone::Renderer::MetaSize2D::Viewport(),
	.samples = 1,
	.mipLevels = 1,
	.depth = 1,
	.arrayLayers = 1,
	.format = Grindstone::GraphicsAPI::Format::D32_SFLOAT,
	.imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D,
	.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUOnly,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::Sampled | Grindstone::GraphicsAPI::ImageUsageFlags::DepthStencil
};

void Grindstone::Editor::RendererFeatures::SelectionOutline::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	outlinePipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/editor/selectionOutline");

	Grindstone::GraphicsAPI::Sampler::CreateInfo screenSamplerCreateInfo{
	screenSamplerCreateInfo.debugName = "Screen Sampler",
	screenSamplerCreateInfo.options = {
			.wrapModeU = GraphicsAPI::TextureWrapMode::ClampToBorder,
			.wrapModeV = GraphicsAPI::TextureWrapMode::ClampToBorder,
			.wrapModeW = GraphicsAPI::TextureWrapMode::ClampToBorder,
			.minFilter = GraphicsAPI::TextureFilter::Nearest,
			.magFilter = GraphicsAPI::TextureFilter::Nearest,
			.anistropy = 0
		}
	};

	screenSampler = engineCore.GetGraphicsCore()->GetOrCreateSampler(screenSamplerCreateInfo);
}

void Grindstone::Editor::RendererFeatures::SelectionOutline::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	auto gbufferDataResponse = context.blackboard.GetValue<Grindstone::Renderer::GbufferData>();
	if (gbufferDataResponse.HasError()) {
		GPRINT_ERROR(LogSource::Rendering, "SelectionOutline Unable to get Gbuffer: {}", gbufferDataResponse.GetError());
		return;
	}
	Grindstone::Renderer::GbufferData gbufferData = gbufferDataResponse.GetValue();

	Grindstone::Renderer::RenderGraphBuilderResourceRef colorImageRef = context.colorRef;
	Grindstone::Renderer::RenderGraphBuilderResourceRef depthImageRef = gbufferData.depthRef;

	using SelectionGeometryReturn = std::tuple<Renderer::RenderGraphBuilderResourceRef, Renderer::RenderGraphBuilderResourceRef>;
	auto [selectionIdsRef, selectionDepthRef] = renderGraphBuilder.CreateGraphicsPass<SelectionGeometryReturn>(
		"Selection Geometry",
		Renderer::MetaRect::Swapchain(),
		[](Renderer::GraphicsRenderGraphBuilderPass<SelectionGeometryReturn>& pass) -> SelectionGeometryReturn {
			GraphicsAPI::ClearColor clearColor(UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX);
			GraphicsAPI::ClearDepthStencil clearDepthStencil{};
			clearDepthStencil.depth = 1.0f;
			clearDepthStencil.stencil = 0;

			auto selectionIdsRef = pass.WriteColorAttachment(resource, GraphicsAPI::LoadOp::Clear, clearColor);
			auto selectionDepthRef = pass.WriteDepthStencilAttachment(depthResourceDesc, GraphicsAPI::LoadOp::Clear, clearDepthStencil);
			return { selectionIdsRef, selectionDepthRef };
		},
		[](
			Grindstone::Math::IntRect2D rect,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			SelectionGeometryReturn& outputRef
		) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			engineCore.assetRendererManager->SetEngineDescriptorSet(cxt.globalDescriptorSet);

			const Grindstone::Rendering::GeometryRenderStats stats = engineCore.assetRendererManager->RenderQueue("Selection Geometry for Outline", cmd, cxt.cameraViewData, cxtSet->GetEntityRegistry(), selectionGeometryRenderPassKey);
			// TODO: RenderGraph 2.0 - pushRenderingStatsCallback(stats);
		}
	);

	renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		"Selection Outline",
		Renderer::MetaRect::Swapchain(),
		[this, colorImageRef, depthImageRef, selectionIdsRef, selectionDepthRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) {
			renderPass.ReadExternalSampler(screenSampler);
			renderPass.ReadSampledImage(selectionIdsRef);
			renderPass.ReadSampledImage(selectionDepthRef);
			renderPass.ReadSampledImage(depthImageRef);
			// TODO: There should be a way to recover the image description from the reference.
			Renderer::RenderGraphBuilderResourceRef output = renderPass.ReadWriteColorAttachment(colorImageRef);

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

			Grindstone::GraphicsPipelineAsset* outlinePipelineSetAsset = outlinePipelineSet.Get();
			if (outlinePipelineSetAsset == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::PipelineLayout* outlinePipelineSetLayout = outlinePipelineSetAsset->GetFirstPassPipelineLayout();
			Grindstone::GraphicsAPI::GraphicsPipeline* outlinePipelineSet = outlinePipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
			if (outlinePipelineSet == nullptr) {
				return;
			}

			cmd->BindGraphicsPipeline(outlinePipelineSet);
			cmd->DrawVertices(3, 0, 1, 0);
		}
	);
}
