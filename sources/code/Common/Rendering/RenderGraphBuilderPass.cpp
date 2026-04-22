#include "RenderGraphBuilderPass.hpp"
#include "RenderGraphBuilder.hpp"

// ============================================
// Pipeline Pass (Graphics and Compute)
// ============================================

// Storage

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadImage(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	imageRefs.emplace_back( inputHandle, AccessType::Read );
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadWriteImage(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	imageRefs.emplace_back( ref, AccessType::ReadWrite );
	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::WriteImage(ImageDescription resource) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddImage(resource, passIndex);
	imageRefs.emplace_back( ref, AccessType::Write );
	return ref;
}

// Buffers

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadBuffer(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	bufferRefs.emplace_back( inputHandle, AccessType::Read );
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadWriteBuffer(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	bufferRefs.emplace_back( ref, AccessType::ReadWrite );
	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::WriteBuffer(BufferDescription resource) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddBuffer(resource, passIndex);
	bufferRefs.emplace_back( ref, AccessType::Write );
	return ref;
}

// ============================================
// Graphics Pass
// ============================================

// Color Attachment

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadWriteColorAttachment(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	imageRefs.emplace_back( ref, AccessType::ReadWrite );
	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteColorAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddImage(resource, passIndex);
	imageRefs.emplace_back( ref, AccessType::Write );
	return ref;
}

// Depth Stencil Attachment

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadWriteDepthStencilAttachment(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	imageRefs.emplace_back( ref, AccessType::ReadWrite );
	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteDepthStencilAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddImage(resource, passIndex);
	imageRefs.emplace_back( ref, AccessType::Write );
	return ref;
}

void Grindstone::Renderer::TransferRenderGraphBuilderPass::AddImageTransfer(BuilderImageTransfer transfer) {
	imageTransfers.push_back(transfer);
}

void Grindstone::Renderer::TransferRenderGraphBuilderPass::AddBufferTransfer(BuilderBufferTransfer transfer) {
	bufferTransfers.push_back(transfer);
}

void Grindstone::Renderer::PresentRenderGraphBuilderPass::SetPresentationImage(RenderGraphBuilderResourceRef targetImage) {
	this->targetImage = targetImage;
}
