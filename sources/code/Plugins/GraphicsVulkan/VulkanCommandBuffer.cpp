#include <cstring>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderPass.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanComputePipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanVertexArrayObject.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanIndexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanCommandBuffer.hpp"

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
	Base::GraphicsPipeline* graphicsPipeline,
	Base::DescriptorSet** descriptorSets,
	uint32_t descriptorSetCount
) {
	Vulkan::GraphicsPipeline *vkPipeline = static_cast<Vulkan::GraphicsPipeline *>(graphicsPipeline);
	BindDescriptorSet(vkPipeline->GetGraphicsPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSets, descriptorSetCount);
}

void Vulkan::CommandBuffer::BindComputeDescriptorSet(
	Base::ComputePipeline* graphicsPipeline,
	Base::DescriptorSet** descriptorSets,
	uint32_t descriptorSetCount
) {
	Vulkan::ComputePipeline* vkPipeline = static_cast<Vulkan::ComputePipeline*>(graphicsPipeline);
	BindDescriptorSet(vkPipeline->GetComputePipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE, descriptorSets, descriptorSetCount);
}

void Vulkan::CommandBuffer::BindDescriptorSet(
	VkPipelineLayout pipelineLayout,
	VkPipelineBindPoint bindPoint,
	Base::DescriptorSet** descriptorSets,
	uint32_t descriptorSetCount
) {
	std::vector<VkDescriptorSet> vkDescriptorSets;
	vkDescriptorSets.reserve(descriptorSetCount);
	for (uint32_t i = 0; i < descriptorSetCount; i++) {
		auto descriptorPtr = descriptorSets[i];
		VkDescriptorSet desc = descriptorPtr != nullptr
			? static_cast<Vulkan::DescriptorSet*>(descriptorPtr)->GetDescriptorSet()
			: nullptr;
		vkDescriptorSets.push_back(desc);
	}

	vkCmdBindDescriptorSets(
		commandBuffer,
		bindPoint,
		pipelineLayout,
		0,
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

void Vulkan::CommandBuffer::BlitDepthImage(GraphicsAPI::DepthTarget* src, GraphicsAPI::DepthTarget* dst) {
	Vulkan::DepthTarget* vkSrc = static_cast<Vulkan::DepthTarget*>(src);
	Vulkan::DepthTarget* vkDst = static_cast<Vulkan::DepthTarget*>(dst);

	VkImageLayout srcLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	VkImageLayout dstLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { static_cast<int>(vkSrc->GetWidth()), static_cast<int>(vkSrc->GetHeight()), 1};
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { static_cast<int>(vkDst->GetWidth()), static_cast<int>(vkDst->GetHeight()), 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

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
	scissor.offset = { 0, 0 };
	scissor.extent.width = width;
	scissor.extent.height = height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Vulkan::CommandBuffer::BindGraphicsPipeline(Base::GraphicsPipeline* pipeline) {
	Vulkan::GraphicsPipeline* vulkanPipeline = static_cast<Vulkan::GraphicsPipeline*>(pipeline);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetGraphicsPipeline());
}

void Vulkan::CommandBuffer::BindComputePipeline(Base::ComputePipeline* pipeline) {
	Vulkan::ComputePipeline * vulkanPipeline = static_cast<Vulkan::ComputePipeline*>(pipeline);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipeline->GetComputePipeline());
}

void Vulkan::CommandBuffer::BindVertexArrayObject(Base::VertexArrayObject* vertexArrayObject) {
	Vulkan::VertexArrayObject* vkVao = static_cast<Vulkan::VertexArrayObject*>(vertexArrayObject);
	auto& vkVertexBuffers = vkVao->GetVertexBuffers();

	BindVertexBuffers(vkVertexBuffers.data(), static_cast<uint32_t>(vkVertexBuffers.size()));
	BindIndexBuffer(vkVao->GetIndexBuffer());
}

void Vulkan::CommandBuffer::BindVertexBuffers(Base::VertexBuffer** vertexBuffers, uint32_t vertexBufferCount) {
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;
	buffers.resize(vertexBufferCount);
	offsets.resize(vertexBufferCount);

	for (uint32_t i = 0; i < vertexBufferCount; ++i) {
		Vulkan::VertexBuffer *vb = static_cast<Vulkan::VertexBuffer *>(vertexBuffers[i]);
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

void Vulkan::CommandBuffer::BindIndexBuffer(Base::IndexBuffer* indexBuffer) {
	Vulkan::IndexBuffer *vulkanIndexBuffer = static_cast<Vulkan::IndexBuffer *>(indexBuffer);
	vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer->GetBuffer(), 0, vulkanIndexBuffer->Is32Bit() ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
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

void Vulkan::CommandBuffer::WaitForComputeMemoryBarrier(GraphicsAPI::RenderTarget* renderTarget, bool shouldMakeWritable) {
	Vulkan::RenderTarget* vulkanRenderTarget = static_cast<Vulkan::RenderTarget*>(renderTarget);
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = shouldMakeWritable ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout = shouldMakeWritable ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.image = vulkanRenderTarget->GetImage();
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.srcAccessMask = shouldMakeWritable ? VK_ACCESS_SHADER_READ_BIT : VK_ACCESS_SHADER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = shouldMakeWritable ? VK_ACCESS_SHADER_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier
	);
}

void Vulkan::CommandBuffer::EndCommandBuffer() {
	vkEndCommandBuffer(commandBuffer);
}
