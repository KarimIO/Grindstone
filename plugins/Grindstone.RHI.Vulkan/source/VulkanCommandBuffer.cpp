#include <cstring>

#include <EngineCore/Logger.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanGraphicsPipeline.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanComputePipeline.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFramebuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanVertexArrayObject.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanBuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanImage.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSet.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSetLayout.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCommandBuffer.hpp>

PFN_vkCmdBeginDebugUtilsLabelEXT cmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT cmdEndDebugUtilsLabelEXT;

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

void Vulkan::CommandBuffer::SetupDebugLabelUtils(VkInstance instance) {
	cmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
	cmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
}

VkCommandBuffer Vulkan::CommandBuffer::GetCommandBuffer()	{
	return commandBuffer;
}

Vulkan::CommandBuffer::CommandBuffer(const CreateInfo& createInfo) {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = Vulkan::Core::Get().GetGraphicsCommandPool();
	allocInfo.level = createInfo.secondaryInfo.isSecondary
		? VK_COMMAND_BUFFER_LEVEL_SECONDARY
		: VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Failed to allocate command buffers!");
	}

	if (createInfo.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_COMMAND_BUFFER, commandBuffer, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Command Buffer!");
	}

	secondaryInfo = createInfo.secondaryInfo;

	beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (createInfo.secondaryInfo.isSecondary) {
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		VkCommandBufferInheritanceInfo inheritenceInfo = {};
		Vulkan::Framebuffer* framebuffer = static_cast<Vulkan::Framebuffer *>(createInfo.secondaryInfo.framebuffer);
		Vulkan::RenderPass* renderPass = static_cast<Vulkan::RenderPass *>(createInfo.secondaryInfo.renderPass);
		inheritenceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritenceInfo.framebuffer = framebuffer->GetFramebuffer();
		inheritenceInfo.renderPass = renderPass->GetRenderPassHandle();
		inheritenceInfo.occlusionQueryEnable = VK_FALSE;
		inheritenceInfo.pipelineStatistics = 0;
		inheritenceInfo.pNext = nullptr;
		inheritenceInfo.subpass = 0;
		beginInfo.pInheritanceInfo = &inheritenceInfo;
	}
}

Vulkan::CommandBuffer::~CommandBuffer() {
}

void Vulkan::CommandBuffer::BeginCommandBuffer() {
	vkResetCommandBuffer(commandBuffer, 0);
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void Vulkan::CommandBuffer::BindRenderPass(
	GraphicsAPI::RenderPass* renderPass,
	GraphicsAPI::Framebuffer* framebuffer,
	uint32_t width,
	uint32_t height,
	ClearColorValue* colorClearValues,
	uint32_t colorClearCount,
	ClearDepthStencil depthStencilClearValue
) {
	Vulkan::RenderPass *vulkanRenderPass = static_cast<Vulkan::RenderPass*>(renderPass);
	Vulkan::Framebuffer *vulkanFramebuffer = static_cast<Vulkan::Framebuffer*>(framebuffer);
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkanRenderPass->GetRenderPassHandle();
	renderPassInfo.framebuffer = vulkanFramebuffer->GetFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { width, height };

	size_t clearColorCount = colorClearCount;
	clearColorCount += depthStencilClearValue.hasDepthStencilAttachment ? 1 : 0;

	std::vector<VkClearValue> clearColor;
	clearColor.resize(clearColorCount);
	for (size_t i = 0; i < colorClearCount; i++) {
		std::memcpy(&clearColor[i].color, &colorClearValues[i], sizeof(VkClearColorValue));
	}

	if (depthStencilClearValue.hasDepthStencilAttachment) {
		clearColor[colorClearCount].depthStencil = {
			depthStencilClearValue.depth,
			depthStencilClearValue.stencil
		};
	}

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();

	VkDebugUtilsLabelEXT labelInfo{};
	labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	labelInfo.pLabelName = renderPass->GetDebugName();
	labelInfo.pNext = nullptr;

	if (labelInfo.color != nullptr) {
		const float* debugColor = renderPass->GetDebugColor();
		labelInfo.color[0] = debugColor[0];
		labelInfo.color[1] = debugColor[1];
		labelInfo.color[2] = debugColor[2];
		labelInfo.color[3] = debugColor[3];
	}

	cmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Vulkan::CommandBuffer::UnbindRenderPass() {
	vkCmdEndRenderPass(commandBuffer);
	cmdEndDebugUtilsLabelEXT(commandBuffer);
}

void Vulkan::CommandBuffer::BeginDebugLabelSection(const char* name, float color[4]) {
	VkDebugUtilsLabelEXT labelInfo{};
	labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	labelInfo.pLabelName = name;
	labelInfo.pNext = nullptr;

	if (color != nullptr) {
		labelInfo.color[0] = color[0];
		labelInfo.color[1] = color[1];
		labelInfo.color[2] = color[2];
		labelInfo.color[3] = color[3];
	}

	cmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
}

void Vulkan::CommandBuffer::EndDebugLabelSection() {
	cmdEndDebugUtilsLabelEXT(commandBuffer);
}

void Vulkan::CommandBuffer::BindGraphicsDescriptorSet(
	const Base::GraphicsPipeline* graphicsPipeline,
	const Base::DescriptorSet* const * descriptorSets,
	uint32_t descriptorSetOffset,
	uint32_t descriptorSetCount
) {
	const Vulkan::GraphicsPipeline *vkPipeline = static_cast<const Vulkan::GraphicsPipeline *>(graphicsPipeline);
	BindDescriptorSet(vkPipeline->GetGraphicsPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSets, descriptorSetOffset, descriptorSetCount);
}

void Vulkan::CommandBuffer::BindComputeDescriptorSet(
	const Base::ComputePipeline* computePipeline,
	const Base::DescriptorSet* const * descriptorSets,
	uint32_t descriptorSetOffset,
	uint32_t descriptorSetCount
) {
	const Vulkan::ComputePipeline* vkPipeline = static_cast<const Vulkan::ComputePipeline*>(computePipeline);
	BindDescriptorSet(vkPipeline->GetComputePipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE, descriptorSets, descriptorSetOffset, descriptorSetCount);
}

void Vulkan::CommandBuffer::BindDescriptorSet(
	VkPipelineLayout pipelineLayout,
	VkPipelineBindPoint bindPoint,
	const Base::DescriptorSet* const * descriptorSets,
	uint32_t descriptorSetOffset,
	uint32_t descriptorSetCount
) {
	std::vector<VkDescriptorSet> vkDescriptorSets;
	vkDescriptorSets.reserve(descriptorSetCount);
	for (uint32_t i = 0; i < descriptorSetCount; i++) {
		auto descriptorPtr = descriptorSets[i];
		VkDescriptorSet desc = descriptorPtr != nullptr
			? static_cast<const Vulkan::DescriptorSet*>(descriptorPtr)->GetDescriptorSet()
			: nullptr;
		vkDescriptorSets.push_back(desc);
	}

	vkCmdBindDescriptorSets(
		commandBuffer,
		bindPoint,
		pipelineLayout,
		descriptorSetOffset,
		static_cast<uint32_t>(vkDescriptorSets.size()),
		vkDescriptorSets.data(),
		0,
		nullptr
	);
}

void Vulkan::CommandBuffer::BindCommandBuffers(
	GraphicsAPI::CommandBuffer** commandBuffers, uint32_t commandBuffersCount
) {
	std::vector<VkCommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffersCount);
	for (size_t i = 0; i < commandBuffersCount; i++) {
		VkCommandBuffer cmd = static_cast<Vulkan::CommandBuffer *>(commandBuffers[i])->GetCommandBuffer();
		vkCommandBuffers.push_back(cmd);
	}

	vkCmdExecuteCommands(
		commandBuffer,
		static_cast<uint32_t>(vkCommandBuffers.size()),
		vkCommandBuffers.data()
	);
}

void Vulkan::CommandBuffer::BlitImage(
	GraphicsAPI::Image* src,
	GraphicsAPI::Image* dst,
	Grindstone::GraphicsAPI::ImageLayout oldLayout,
	Grindstone::GraphicsAPI::ImageLayout newLayout,
	uint32_t width, uint32_t height, uint32_t depth
) {
	Vulkan::Image* vkSrc = static_cast<Vulkan::Image*>(src);
	Vulkan::Image* vkDst = static_cast<Vulkan::Image*>(dst);

	VkImageLayout srcLayout = TranslateImageLayoutToVulkan(oldLayout);
	VkImageLayout dstLayout = TranslateImageLayoutToVulkan(newLayout);

	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = {
		static_cast<int32_t>(width),
		static_cast<int32_t>(height),
		static_cast<int32_t>(depth)
	};
	blit.srcSubresource.aspectMask = vkSrc->GetAspect();
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = vkSrc->GetArrayLayers();
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = {
		static_cast<int32_t>(width),
		static_cast<int32_t>(height),
		static_cast<int32_t>(depth)
	};
	blit.dstSubresource.aspectMask = vkDst->GetAspect();
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = vkDst->GetArrayLayers();

	VkFilter filter = VkFilter::VK_FILTER_NEAREST;

	vkCmdBlitImage(commandBuffer, vkSrc->GetImage(), srcLayout, vkDst->GetImage(), dstLayout, 1, &blit, filter);
}

void Vulkan::CommandBuffer::SetDepthBias(float biasConstantFactor, float biasSlopeFactor) {
	vkCmdSetDepthBias(
		commandBuffer,
		biasConstantFactor,
		0.0f,
		biasSlopeFactor
	);
}

void Vulkan::CommandBuffer::SetViewport(float offsetX, float offsetY, float width, float height, float depthMin, float depthMax) {
	VkViewport viewport{};
	viewport.x = offsetX;
	viewport.y = offsetY;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = depthMin;
	viewport.maxDepth = depthMax;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void Vulkan::CommandBuffer::SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) {
	VkRect2D scissor{};
	scissor.offset = { offsetX, offsetY };
	scissor.extent.width = width;
	scissor.extent.height = height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Vulkan::CommandBuffer::BindGraphicsPipeline(const Base::GraphicsPipeline* pipeline) {
	const Vulkan::GraphicsPipeline* vulkanPipeline = static_cast<const Vulkan::GraphicsPipeline*>(pipeline);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetGraphicsPipeline());
}

void Vulkan::CommandBuffer::BindComputePipeline(const Base::ComputePipeline* pipeline) {
	const Vulkan::ComputePipeline * vulkanPipeline = static_cast<const Vulkan::ComputePipeline*>(pipeline);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipeline->GetComputePipeline());
}

void Vulkan::CommandBuffer::BindVertexArrayObject(const Base::VertexArrayObject* vertexArrayObject) {
	const Vulkan::VertexArrayObject* vkVao = static_cast<const Vulkan::VertexArrayObject*>(vertexArrayObject);
	const auto& vkVertexBuffers = vkVao->GetVertexBuffers();

	BindVertexBuffers(vkVertexBuffers.data(), static_cast<uint32_t>(vkVertexBuffers.size()));
	BindIndexBuffer(vkVao->GetIndexBuffer());
}

void Vulkan::CommandBuffer::BindVertexBuffers(const Base::Buffer* const * vertexBuffers, uint32_t vertexBufferCount) {
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;
	buffers.resize(vertexBufferCount);
	offsets.resize(vertexBufferCount);

	for (uint32_t i = 0; i < vertexBufferCount; ++i) {
		const Vulkan::Buffer *vb = static_cast<const Vulkan::Buffer*>(vertexBuffers[i]);
		buffers[i] = vb->GetBuffer();
		offsets[i] = 0;
	}

	vkCmdBindVertexBuffers(
		commandBuffer,
		0,
		vertexBufferCount,
		buffers.data(),
		offsets.data()
	);
}

void Vulkan::CommandBuffer::BindIndexBuffer(Base::Buffer* indexBuffer) {
	Vulkan::Buffer* vulkanIndexBuffer = static_cast<Vulkan::Buffer*>(indexBuffer);

	// TODO: Where can I get this from?
	VkIndexType indexType = false ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
	vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer->GetBuffer(), 0, indexType);
}

void Vulkan::CommandBuffer::DrawVertices(uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) {
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, vertexOffset, firstInstance);
}

void Vulkan::CommandBuffer::DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) {
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Vulkan::CommandBuffer::DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
	vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void Vulkan::CommandBuffer::PipelineBarrier(
	GraphicsAPI::PipelineStageBit srcPipelineStageMask,
	GraphicsAPI::PipelineStageBit dstPipelineStageMask,
	const GraphicsAPI::BufferBarrier* bufferBarriers, uint32_t bufferBarrierCount,
	const GraphicsAPI::ImageBarrier* imageBarriers, uint32_t imageBarrierCount
) {
	std::vector<VkBufferMemoryBarrier> vkBufferBarriers;
	std::vector<VkImageMemoryBarrier> vkImageBarriers;

	for (uint32_t index = 0; index < imageBarrierCount; ++index) {
		const GraphicsAPI::ImageBarrier& barrier = imageBarriers[index];

		Vulkan::Image* vulkanImage = static_cast<Vulkan::Image*>(barrier.image);

		vkImageBarriers.emplace_back(
			VkImageMemoryBarrier {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = (VkAccessFlags)(barrier.srcAccess),
				.dstAccessMask = (VkAccessFlags)(barrier.dstAccess),
				.oldLayout = TranslateImageLayoutToVulkan(barrier.oldLayout),
				.newLayout = TranslateImageLayoutToVulkan(barrier.newLayout),
				.image = vulkanImage->GetImage(),
				.subresourceRange = {
					.aspectMask = TranslateImageAspectBitsToVulkan(barrier.imageAspect),
					.baseMipLevel = barrier.baseMipLevel,
					.levelCount = barrier.levelCount,
					.baseArrayLayer = barrier.baseArrayLayer,
					.layerCount = barrier.layerCount,
				}
			}
		);
	}

	for (uint32_t index = 0; index < bufferBarrierCount; ++index) {
		const GraphicsAPI::BufferBarrier& barrier = bufferBarriers[index];

		Vulkan::Buffer* vulkanBuffer = static_cast<Vulkan::Buffer*>(barrier.buffer);

		vkBufferBarriers.emplace_back(
			VkBufferMemoryBarrier {
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				.srcAccessMask = (VkAccessFlags)(barrier.srcAccess),
				.dstAccessMask = (VkAccessFlags)(barrier.dstAccess),
				.buffer = vulkanBuffer->GetBuffer(),
				.offset = static_cast<VkDeviceSize>(barrier.offset),
				.size = static_cast<VkDeviceSize>(barrier.size)
			}
		);
	}

	VkPipelineStageFlagBits vkSrcPipelineStage = TranslatePipelineStageToVulkan(srcPipelineStageMask);
	VkPipelineStageFlagBits vkDstPipelineStage = TranslatePipelineStageToVulkan(dstPipelineStageMask);

	vkCmdPipelineBarrier(
		commandBuffer,
		vkSrcPipelineStage,
		vkDstPipelineStage,
		0,
		0, nullptr,
		static_cast<uint32_t>(vkBufferBarriers.size()),
		vkBufferBarriers.data(),
		static_cast<uint32_t>(vkImageBarriers.size()),
		vkImageBarriers.data()
	);
}

void Vulkan::CommandBuffer::EndCommandBuffer() {
	vkEndCommandBuffer(commandBuffer);
}
