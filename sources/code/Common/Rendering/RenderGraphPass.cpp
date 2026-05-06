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

struct ImageUsageSyncInfo {
	Grindstone::GraphicsAPI::PipelineStageBit stageMask;
	Grindstone::GraphicsAPI::AccessFlags accessMask;
	Grindstone::GraphicsAPI::ImageLayout layout;
};

static Grindstone::GraphicsAPI::PipelineStageBit DeriveShaderStageMask(Grindstone::GraphicsAPI::ShaderStageBit visibility) {
	Grindstone::GraphicsAPI::PipelineStageBit mask = Grindstone::GraphicsAPI::PipelineStageBit::None;

	if (static_cast<uint8_t>(visibility) & static_cast<uint8_t>(Grindstone::GraphicsAPI::ShaderStageBit::Vertex))
		mask |= Grindstone::GraphicsAPI::PipelineStageBit::VertexShader;

	if (static_cast<uint8_t>(visibility) & static_cast<uint8_t>(Grindstone::GraphicsAPI::ShaderStageBit::Fragment))
		mask |= Grindstone::GraphicsAPI::PipelineStageBit::FragmentShader;

	if (static_cast<uint8_t>(visibility) & static_cast<uint8_t>(Grindstone::GraphicsAPI::ShaderStageBit::Compute))
		mask |= Grindstone::GraphicsAPI::PipelineStageBit::ComputeShader;

	return mask;
}

static Grindstone::GraphicsAPI::ImageLayout DeriveDepthStencilLayout(
	bool isDepth, bool isStencil,
	bool isRead, bool isWrite
) {
	// TODO: Fix Stencil and DepthStencil variatns
	if (isDepth && isStencil) {
		if (isRead && !isWrite) return Grindstone::GraphicsAPI::ImageLayout::DepthRead;
		return Grindstone::GraphicsAPI::ImageLayout::DepthWrite;
	}

	if (isDepth) {
		if (isRead && !isWrite) return Grindstone::GraphicsAPI::ImageLayout::DepthRead;
		return Grindstone::GraphicsAPI::ImageLayout::DepthWrite;
	}

	if (isStencil) {
		if (isRead && !isWrite) return Grindstone::GraphicsAPI::ImageLayout::DepthRead;
		return Grindstone::GraphicsAPI::ImageLayout::DepthWrite;
	}

	return Grindstone::GraphicsAPI::ImageLayout::Undefined;
}

static ImageUsageSyncInfo DeriveImageSyncInfo(
	RenderGraphImageUsage usage,
	Grindstone::GraphicsAPI::ShaderStageBit visibility = Grindstone::GraphicsAPI::ShaderStageBit::AllGraphics
) {
	const bool isRead = Any(usage & RenderGraphImageUsage::Read);
	const bool isWrite = Any(usage & RenderGraphImageUsage::Write);
	const bool isSampled = Any(usage & RenderGraphImageUsage::Sampled);
	const bool isStorage = Any(usage & RenderGraphImageUsage::Storage);
	const bool isColor = Any(usage & RenderGraphImageUsage::ColorAttachment);
	const bool isDepth = Any(usage & RenderGraphImageUsage::DepthAttachment);
	const bool isStencil = Any(usage & RenderGraphImageUsage::StencilAttachment);
	const bool isSubpassInput = Any(usage & RenderGraphImageUsage::SubpassInputAttachment);
	const bool isTransfer = Any(usage & RenderGraphImageUsage::Transfer);

	Grindstone::GraphicsAPI::PipelineStageBit stageMask = Grindstone::GraphicsAPI::PipelineStageBit::None;
	Grindstone::GraphicsAPI::AccessFlags accessMask = Grindstone::GraphicsAPI::AccessFlags::None;
	Grindstone::GraphicsAPI::ImageLayout layout = Grindstone::GraphicsAPI::ImageLayout::Undefined;

	// ── Transfer ──────────────────────────────────────────────────────────
	if (isTransfer) {
		if (isRead) {
			stageMask |= Grindstone::GraphicsAPI::PipelineStageBit::Transfer;
			accessMask |= Grindstone::GraphicsAPI::AccessFlags::TransferRead;
			layout = Grindstone::GraphicsAPI::ImageLayout::TransferSrc;
		}
		if (isWrite) {
			stageMask |= Grindstone::GraphicsAPI::PipelineStageBit::Transfer;
			accessMask |= Grindstone::GraphicsAPI::AccessFlags::TransferWrite;
			layout = Grindstone::GraphicsAPI::ImageLayout::TransferDst;
		}
		// Transfer is mutually exclusive with other usage types
		// so return early — no further flag combinations are valid
		return { stageMask, accessMask, layout };
	}

	// ── Color Attachment ───────────────────────────────────────────────────
	if (isColor) {
		stageMask |= Grindstone::GraphicsAPI::PipelineStageBit::ColorAttachmentOutput;

		if (isRead)  accessMask |= Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentRead;
		if (isWrite) accessMask |= Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite;

		layout = isRead
			? Grindstone::GraphicsAPI::ImageLayout::Undefined  // ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT blend read + write
			: Grindstone::GraphicsAPI::ImageLayout::ColorAttachment;             // write only
	}

	// ── Depth / Stencil Attachment ─────────────────────────────────────────
	if (isDepth || isStencil) {
		// Stage: early + late fragment tests cover all depth/stencil access
		stageMask |= Grindstone::GraphicsAPI::PipelineStageBit::EarlyFragmentTests
			| Grindstone::GraphicsAPI::PipelineStageBit::LateFragmentTests;

		if (isRead)  accessMask |= Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentRead; // DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if (isWrite) accessMask |= Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite; // DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// Layout is determined by the combination of depth+stencil × read+write
		layout = DeriveDepthStencilLayout(isDepth, isStencil, isRead, isWrite);
	}

	// ── Sampled (shader read via sampler) ─────────────────────────────────
	if (isSampled && isRead) {
		stageMask |= DeriveShaderStageMask(visibility);
		accessMask |= Grindstone::GraphicsAPI::AccessFlags::ShaderRead; // SHADER_SAMPLED_READ_BIT;

		// Only set layout here if no attachment flags are present
		// Attachment flags take precedence and produce read-only variants
		if (!isDepth && !isStencil && !isColor && !isSubpassInput) {
			layout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead;
		}
		// If combined with depth/stencil, layout stays as depth read-only variant
		// (DEPTH_READ_ONLY_OPTIMAL also permits shader sampling)
	}

	// ── Storage Image (shader read/write without sampler) ─────────────────
	if (isStorage) {
		stageMask |= DeriveShaderStageMask(visibility);

		if (isRead)  accessMask |= Grindstone::GraphicsAPI::AccessFlags::ShaderRead; // SHADER_STORAGE_READ_BIT;
		if (isWrite) accessMask |= Grindstone::GraphicsAPI::AccessFlags::ShaderWrite; // SHADER_STORAGE_WRITE_BIT;

		layout = Grindstone::GraphicsAPI::ImageLayout::General;
	}

	// ── Subpass Input Attachment ───────────────────────────────────────────
	if (isSubpassInput && isRead) {
		stageMask |= Grindstone::GraphicsAPI::PipelineStageBit::FragmentShader;
		accessMask |= Grindstone::GraphicsAPI::AccessFlags::InputAttachmentRead;
		layout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead;
	}

	if (isStorage && (isColor || isDepth || isStencil)) {
		layout = Grindstone::GraphicsAPI::ImageLayout::General;
	}

	return { stageMask, accessMask, layout };
}

void Grindstone::Renderer::PipelineRenderGraphPass::RealizeResources(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
	std::string passDescriptorSetName = name + " Pass Descriptor Set";
	std::string passDescriptorSetLayoutName = passDescriptorSetName + " Layout";

	std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> layoutBindings;
	std::vector<Grindstone::GraphicsAPI::DescriptorSet::Binding> bindings;

	for (uint32_t i = 0; i < static_cast<uint32_t>(imageDescs.size()); ++i) {
		const Grindstone::Renderer::PassImageDesc& imageDesc = imageDescs[i];
		ResourceId imageRef = imageDesc.ref.GetResourceIndex();
		Grindstone::GraphicsAPI::Image* image = frameResources.GetImage(imageRef);

		if (imageDesc.IsShaderInput()) {
			layoutBindings.emplace_back(
				GraphicsAPI::DescriptorSetLayout::Binding{
					.bindingId = i,
					.count = 1,
					.type = Grindstone::GraphicsAPI::BindingType::SampledImage,
					.stages = Grindstone::GraphicsAPI::ShaderStageBit::All
				}
			);
			bindings.emplace_back(GraphicsAPI::DescriptorSet::Binding::SampledImage(image));
		}


		auto [prevLayout, prevAccessFlags, prevPipelineStage] = frameResources.GetLayout(imageRef);
		auto [newStageMask, newAccessMask, newImageLayout] = DeriveImageSyncInfo(imageDesc.usage);

		Grindstone::GraphicsAPI::ImageAspectBits imageAspect = Any(imageDesc.usage & RenderGraphImageUsage::DepthAttachment)
			? Grindstone::GraphicsAPI::ImageAspectBits::Depth
			: Grindstone::GraphicsAPI::ImageAspectBits::Color;

		Grindstone::GraphicsAPI::ImageBarrier imageBarrier{
			.image = image,
			.srcStageMask = prevPipelineStage,
			.dstStageMask = newStageMask,
			.oldLayout = prevLayout,
			.newLayout = newImageLayout,
			.srcAccess = prevAccessFlags,
			.dstAccess = newAccessMask,
			.imageAspect = imageAspect,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		frameResources.SetLayout(imageRef, newImageLayout, newAccessMask, newStageMask);
		imageBarriers.emplace_back(imageBarrier);
	}

	for (const Grindstone::Renderer::PassBufferDesc& bufferDesc : bufferDescs) {
		/*
		if (bufferDesc.accessType != AccessType::Write) {
			layoutBindings.emplace_back(
				Grindstone::GraphicsAPI::DescriptorSetLayout::Binding{
					.
				}
			);

			bindings.emplace_back(
				Grindstone::GraphicsAPI::DescriptorSet::Binding::
			);
		}
		*/
	}

	std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout*> descriptorSetLayouts{
		context.globalDescriptorSetLayout,
	};

	if (layoutBindings.size() > 0) {
		Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{
			.debugName = passDescriptorSetLayoutName.c_str(),
			.bindings = layoutBindings.data(),
			.bindingCount = static_cast<uint32_t>(layoutBindings.size())
		};

		Grindstone::GraphicsAPI::DescriptorSetLayout* passDescriptorSetLayout = context.graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		Grindstone::GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{
			.debugName = passDescriptorSetName.c_str(),
			.layout = passDescriptorSetLayout,
			.bindings = bindings.data(),
			.bindingCount = static_cast<uint32_t>(bindings.size())
		};

		passDescriptorSet = context.graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
		descriptorSetLayouts.emplace_back(passDescriptorSetLayout);
	}
	else {
		passDescriptorSet = nullptr;
	}

	Grindstone::GraphicsAPI::PipelineLayout::CreateInfo pipelineLayoutCreateInfo{
		.debugName = name.c_str(),
		.descriptorSetLayouts = descriptorSetLayouts.data(),
		.descriptorSetLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
	};

	pipelineLayout = context.graphicsCore->GetOrCreatePipelineLayoutFromCache(pipelineLayoutCreateInfo);
}

void Grindstone::Renderer::ComputeRenderGraphPassBase::PrepareComputePass(Grindstone::Renderer::RenderGraphContext& context) {
	SubmitBarriers(context);

	std::vector<GraphicsAPI::DescriptorSet*> descriptorSets = {
		context.globalDescriptorSet
	};

	if (passDescriptorSet != nullptr) {
		descriptorSets.emplace_back(passDescriptorSet);
	}

	context.commandBuffer->BindComputeDescriptorSet(pipelineLayout, descriptorSets.data(), 0, descriptorSets.size());
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

	std::vector<GraphicsAPI::DescriptorSet*> descriptorSets = {
		context.globalDescriptorSet
	};

	if (passDescriptorSet != nullptr) {
		descriptorSets.emplace_back(passDescriptorSet);
	}

	context.commandBuffer->BindGraphicsDescriptorSet(pipelineLayout, descriptorSets.data(), 0, descriptorSets.size());

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

void Grindstone::Renderer::TransferRenderGraphPass::RealizeResources(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
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

void Grindstone::Renderer::PresentRenderGraphPass::RealizeResources(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
}

void Grindstone::Renderer::PresentRenderGraphPass::Execute(
	Grindstone::Renderer::RenderGraphContext& context,
	Grindstone::Renderer::RenderGraphFrameResources& frameResources
) {
	SubmitBarriers(context);
}
