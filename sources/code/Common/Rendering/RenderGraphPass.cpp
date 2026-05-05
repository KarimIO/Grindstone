#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Rendering/RenderGraphPass.hpp>

using namespace Grindstone::Renderer;

void Grindstone::Renderer::RenderGraphPass::SubmitBarriers(Grindstone::Renderer::RenderGraphContext& context) {
	if (imageBarriers.empty() && bufferBarriers.empty()) {
		return;
	}

	context.commandBuffer->PipelineBarrier(
		bufferBarriers.data(),
		static_cast<uint32_t>(bufferBarriers.size()),
		imageBarriers.data(),
		static_cast<uint32_t>(imageBarriers.size())
	);
}

void Grindstone::Renderer::ComputeRenderGraphPassBase::RealizeResources(Grindstone::Renderer::RenderGraphContext& context) {
}

void Grindstone::Renderer::ComputeRenderGraphPassBase::PrepareComputePass(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);
}

void Grindstone::Renderer::GraphicsRenderGraphPassBase::RealizeResources(Grindstone::Renderer::RenderGraphContext& context) {
}

Grindstone::Math::IntRect2D Grindstone::Renderer::GraphicsRenderGraphPassBase::PrepareGraphicsPass(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
	SubmitBarriers(context);

	Grindstone::Math::IntRect2D renderingArea = metaRenderingArea.Resolve(context.swapchainSize);

	std::vector<GraphicsAPI::RenderAttachment> colorAttachments;
	std::optional<GraphicsAPI::RenderAttachment> depthAttachment;

	for (const PassImageDesc& desc : imageDescs) {
		if (!desc.IsAttachment()) {
			continue;
		}

		GraphicsAPI::Image* image = frameResources.GetImage(desc.ref.resourceIndex);

		if (Any(desc.usage & RenderGraphImageUsage::ColorAttachment)) {
			colorAttachments.emplace_back(
				GraphicsAPI::RenderAttachment{
					.image = image,
					.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
					.loadOp = desc.attachment.loadOp,
					.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
					.clearValue = desc.attachment.clearValue
				}
			);
		}
		else {
			depthAttachment = GraphicsAPI::RenderAttachment{
				.image = image,
				.imageLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.loadOp = desc.attachment.loadOp,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = desc.attachment.clearValue
			};
		}
	}

	context.commandBuffer->BeginRendering(
		name.c_str(),
		renderingArea,
		colorAttachments.data(),
		static_cast<uint32_t>(colorAttachments.size()),
		depthAttachment.has_value() ? &depthAttachment.value() : nullptr
	);

	context.commandBuffer->SetViewport(
		static_cast<float>(renderingArea.offset.x),
		static_cast<float>(renderingArea.offset.y),
		static_cast<float>(renderingArea.extent.x),
		static_cast<float>(renderingArea.extent.y)
	);
	context.commandBuffer->SetScissor(
		renderingArea.offset.x,
		renderingArea.offset.y,
		renderingArea.extent.x,
		renderingArea.extent.y
	);

	return renderingArea;
}

void Grindstone::Renderer::GraphicsRenderGraphPassBase::EndGraphicsPass(Grindstone::Renderer::RenderGraphContext& context) {
	context.commandBuffer->EndRendering();
}

void Grindstone::Renderer::TransferRenderGraphPass::RealizeResources(Grindstone::Renderer::RenderGraphContext& context) {
}

void Grindstone::Renderer::TransferRenderGraphPass::Execute(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
	SubmitBarriers(context);

	for (const ImageTransfer& copy : imageTransfers) {
		GraphicsAPI::ImageAspectBits aspect = GraphicsAPI::ImageAspectBits::Color;
		Grindstone::GraphicsAPI::Image* srcImg = copy.src;
		Grindstone::GraphicsAPI::Image* dstImg = copy.dst;

		context.commandBuffer->BlitImage(
			srcImg, dstImg,
			GraphicsAPI::ImageLayout::TransferSrc,
			GraphicsAPI::ImageLayout::TransferDst,
			copy.filter,
			copy.srcRegion,
			copy.dstRegion
		);
	}

	for (const BufferTransfer& copy : bufferTransfers) {
		GraphicsAPI::Buffer* dstBuffer = copy.dstBuff;
		GraphicsAPI::Buffer* srcBuffer = copy.srcBuff;

		context.commandBuffer->CopyBufferRegion(
			srcBuffer,
			dstBuffer,
			copy.size,
			copy.srcOffset,
			copy.dstOffset
		);
	}
}

void Grindstone::Renderer::PresentRenderGraphPass::RealizeResources(Grindstone::Renderer::RenderGraphContext& context) {
}

void Grindstone::Renderer::PresentRenderGraphPass::Execute(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
	SubmitBarriers(context);
}
