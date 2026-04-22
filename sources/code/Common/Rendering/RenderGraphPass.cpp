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

void Grindstone::Renderer::ComputeRenderGraphPassBase::PrepareComputePass(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);
}

void Grindstone::Renderer::GraphicsRenderGraphPassBase::PrepareGraphicsPass(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);

	context.commandBuffer->BeginRendering(
		name.c_str(),
		renderingArea,
		colorAttachments.data(),
		static_cast<uint32_t>(colorAttachments.size()),
		hasDepthAttachment ? &depthAttachment : nullptr
	);
}

void Grindstone::Renderer::GraphicsRenderGraphPassBase::EndGraphicsPass(Grindstone::Renderer::RenderGraphContext& context) {
	context.commandBuffer->EndRendering();
}

void Grindstone::Renderer::TransferRenderGraphPass::Execute(Grindstone::Renderer::RenderGraphContext& context) {
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

void Grindstone::Renderer::PresentRenderGraphPass::Execute(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);
}
