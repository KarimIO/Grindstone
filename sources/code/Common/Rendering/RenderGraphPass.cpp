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
		static_cast<uint32_t>(bufferBarriers.size(),
		imageBarriers.data(),
		static_cast<uint32_t>(imageBarriers.size()))
	);
}

void Grindstone::Renderer::GraphicsRenderGraphPass::Execute(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);

	Grindstone::Math::IntRect2D rect;

	std::vector<Grindstone::GraphicsAPI::RenderAttachment> colorAttachments;
	Grindstone::GraphicsAPI::RenderAttachment depthAttachment;
	bool hasDepthAttachment = false;

	context.commandBuffer->BeginRendering(
		name.ToString().c_str(),
		rect,
		colorAttachments.data(),
		colorAttachments.size(),
		hasDepthAttachment ? &depthAttachment : nullptr
	);
	executionCallback(context, this);
	context.commandBuffer->EndRendering();
}

void Grindstone::Renderer::ComputeRenderGraphPass::Execute(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);

	executionCallback(context, this);
}

void Grindstone::Renderer::TransferRenderGraphPass::Execute(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);

	for (const ImageTransfer& copy : imageTransfers) {
		GraphicsAPI::ImageAspectBits aspect = GraphicsAPI::ImageAspectBits::Color;
		const Grindstone::GraphicsAPI::Image* srcImg = data->taskGraph->GetTexture(copy.src);
		const Grindstone::GraphicsAPI::Image* dstImg = data->taskGraph->GetTexture(copy.dst);
		context.commandBuffer->BlitImage(
			srcImg, dstImg,
			GraphicsAPI::ImageLayout::TransferSrc,
			GraphicsAPI::ImageLayout::TransferDst,
			srcTex->GetWidth(),
			srcTex->GetHeight(),
			1u
		);
	}

	for (const BufferTransfer& copy : bufferTransfers) {
		const GraphicsAPI::Buffer* dstBuffer = data->taskGraph->GetBuffer(copy.dstBuff);
		const GraphicsAPI::Buffer* srcBuffer = data->taskGraph->GetBuffer(copy.srcBuff);

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
