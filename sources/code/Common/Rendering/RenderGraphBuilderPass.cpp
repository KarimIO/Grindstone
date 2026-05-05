#include "RenderGraphBuilderPass.hpp"
#include "RenderGraphBuilder.hpp"

// ============================================
// Pipeline Pass (Graphics and Compute)
// ============================================

// Storage

void Grindstone::Renderer::ComputeRenderGraphBuilderPassBase::ReadStorageImage(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = inputHandle,
			.usage = RenderGraphImageUsage::StorageRead
		}
	);
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::ComputeRenderGraphBuilderPassBase::ReadWriteStorageImage(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::StorageReadWrite
		}
	);
	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::ComputeRenderGraphBuilderPassBase::WriteStorageImage(ImageDescription resource) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddImage(resource, passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::StorageWrite
		}
	);
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

void Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadSampledImage(RenderGraphBuilderResourceRef inputHandle) {
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = inputHandle,
			.usage = RenderGraphImageUsage::SampledRead
		}
	);
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadWriteColorAttachment(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::ColorAttachmentReadWrite,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Load
			}
		}
	);

	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteColorAttachment(ImageDescription resource, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearColor clearValue) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddImage(resource, passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::ColorAttachmentWrite,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = loadOp,
				.clearValue = clearValue
			}
		}
	);

	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteColorAttachment(RenderGraphBuilderResourceRef resource, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearColor clearValue) {
	RenderGraphBuilderResourceRef ref = resource.FromPass(passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::ColorAttachmentWrite,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = loadOp,
				.clearValue = clearValue
			}
		}
	);

	return ref;
}

// Depth Stencil Attachment

void Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadDepthAttachmentSampled(RenderGraphBuilderResourceRef inputHandle) {
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = inputHandle,
			.usage = RenderGraphImageUsage::DepthAttachmentRead,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Load
			}
		}
	);
}

void Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadDepthAttachment(RenderGraphBuilderResourceRef inputHandle) {
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = inputHandle,
			.usage = RenderGraphImageUsage::DepthAttachmentRead | RenderGraphImageUsage::Sampled,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Load
			}
		}
	);
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::ReadWriteDepthStencilAttachment(Grindstone::Renderer::RenderGraphBuilderResourceRef inputHandle) {
	RenderGraphBuilderResourceRef ref = inputHandle.FromPass(passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::DepthAttachmentReadWrite,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Load
			}
		}
	);

	return ref;
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::GraphicsRenderGraphBuilderPassBase::WriteDepthStencilAttachment(ImageDescription resource, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearDepthStencil clearValue) {
	RenderGraphBuilderResourceRef ref = renderGraphBuilder->AddImage(resource, passIndex);
	imageRefs.emplace_back(
		PassImageDesc{
			.ref = ref,
			.usage = RenderGraphImageUsage::DepthAttachmentWrite,
			.attachment = PassImageDesc::AttachmentMeta {
				.loadOp = loadOp,
				.clearValue = clearValue
			}
		}
	);

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
