#include "RenderGraphBuilderPass.hpp"

// ============================================
// Pipeline Pass (Graphics and Compute)
// ============================================

// Storage

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadImage(Grindstone::Renderer::TGBImageRef inputHandle) {
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::StorageImage, resource)));
}

Grindstone::Renderer::TGBImageRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadWriteImage(Grindstone::Renderer::TGBImageRef inputHandle) {
	readWrites.emplace_back(ResourceReadWrite(inputName, inputName, UnionResource(ResourceType::StorageImage, resource)));
}

Grindstone::Renderer::TGBImageRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::WriteImage(ImageDescription resource) {
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::StorageImage, resource)));
}

// Buffers

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadBuffer(Grindstone::Renderer::TGBBufferRef inputHandle) {
	reads.emplace_back(ResourceRead(inputName, UnionResource(resource)));
}

Grindstone::Renderer::TGBBufferRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadWriteBuffer(Grindstone::Renderer::TGBBufferRef inputHandle) {
	readWrites.emplace_back(ResourceReadWrite(inputName, outputName, UnionResource(resource)));
}

Grindstone::Renderer::TGBBufferRef Grindstone::Renderer::PipelineRenderGraphBuilderPass::WriteBuffer(BufferDescription resource) {
	writes.emplace_back(ResourceWrite(outputName, UnionResource(resource)));
}

// ============================================
// Graphics Pass
// ============================================

// Color Attachment

void Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadColorAttachment(Grindstone::Renderer::TGBImageRef inputHandle) {
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

Grindstone::Renderer::TGBImageRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadWriteColorAttachment(Grindstone::Renderer::TGBImageRef inputHandle) {
	readWrites.emplace_back(ResourceReadWrite(inputName, inputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

Grindstone::Renderer::TGBImageRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteColorAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue) {
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

// Depth Stencil Attachment

void Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadDepthStencilAttachment(Grindstone::Renderer::TGBImageRef inputName) {
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

Grindstone::Renderer::TGBImageRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadWriteDepthStencilAttachment(Grindstone::Renderer::TGBImageRef inputHandle) {
	readWrites.emplace_back(ResourceReadWrite(inputName, outputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

Grindstone::Renderer::TGBImageRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteDepthStencilAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue) {
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

void Grindstone::Renderer::TransferRenderGraphBuilderPass::AddImageTransfer(ImageTransfer transfer) {
	imageTransfers.push_back(transfer);
}

void Grindstone::Renderer::TransferRenderGraphBuilderPass::AddBufferTransfer(BufferTransfer transfer) {
	bufferTransfers.push_back(transfer);
}

void Grindstone::Renderer::PresentRenderGraphBuilderPass::SetPresentationImage(TGBImageRef targetImage) {
	this->targetImage = targetImage;
}
