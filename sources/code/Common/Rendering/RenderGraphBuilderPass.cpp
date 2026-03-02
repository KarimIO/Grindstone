#include "RenderGraphBuilderPass.hpp"

// ============================================
// Pipeline Pass (Graphics and Compute)
// ============================================

// Storage

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadStorageImage(Grindstone::HashedString inputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Compute);
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::StorageImage, resource)));
}

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadWriteStorageImage(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Compute);
	readWrites.emplace_back(ResourceReadWrite(inputName, inputName, UnionResource(ResourceType::StorageImage, resource)));
}

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::WriteStorageImage(Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Compute);
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::StorageImage, resource)));
}

// Buffers

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadBuffer(Grindstone::HashedString inputName, BufferDescription resource) {
	reads.emplace_back(ResourceRead(inputName, UnionResource(resource)));
}

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::ReadWriteBuffer(Grindstone::HashedString inputName, Grindstone::HashedString outputName, BufferDescription resource) {
	readWrites.emplace_back(ResourceReadWrite(inputName, outputName, UnionResource(resource)));
}

void Grindstone::Renderer::PipelineRenderGraphBuilderPass::WriteBuffer(Grindstone::HashedString outputName, BufferDescription resource) {
	writes.emplace_back(ResourceWrite(outputName, UnionResource(resource)));
}

// ============================================
// Compute Pass
// ============================================

void Grindstone::Renderer::ComputeRenderGraphBuilderPass::SetExecutionCallback(ComputeExecutionCallback callback) {
	execution = callback;
}

// ============================================
// Graphics Pass
// ============================================

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::SetExecutionCallback(GraphicsExecutionCallback callback) {
	execution = callback;
}

// Color Attachment

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::ReadColorAttachment(Grindstone::HashedString inputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::ReadWriteColorAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	readWrites.emplace_back(ResourceReadWrite(inputName, inputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::WriteColorAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue) {
	GS_ASSERT(type == GpuQueue::Graphics);
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

// Depth Stencil Attachment

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::ReadDepthStencilAttachment(Grindstone::HashedString inputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::ReadWriteDepthStencilAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	readWrites.emplace_back(ResourceReadWrite(inputName, outputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

void Grindstone::Renderer::GraphicsRenderGraphBuilderPass::WriteDepthStencilAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue) {
	GS_ASSERT(type == GpuQueue::Graphics);
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::DepthAttachment, resource)));
}
