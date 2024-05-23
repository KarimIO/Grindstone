#include "VulkanCommandBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanRenderTarget.hpp"
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
#include <cstring>

PFN_vkCmdBeginDebugUtilsLabelEXT cmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT cmdEndDebugUtilsLabelEXT;

using namespace Grindstone::GraphicsAPI;

void VulkanCommandBuffer::SetupDebugLabelUtils(VkInstance instance) {
	cmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
	cmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer()	{
	return commandBuffer;
}

VulkanCommandBuffer::VulkanCommandBuffer(CommandBuffer::CreateInfo& createInfo) {
	VkDevice device = VulkanCore::Get().GetDevice();

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VulkanCore::Get().GetGraphicsCommandPool();
	allocInfo.level = createInfo.secondaryInfo.isSecondary
		? VK_COMMAND_BUFFER_LEVEL_SECONDARY
		: VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	if (createInfo.debugName != nullptr) {
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_COMMAND_BUFFER, commandBuffer, createInfo.debugName);
	}
	else {
		throw std::runtime_error("Unnamed Command Buffer!");
	}

	secondaryInfo = createInfo.secondaryInfo;

	beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (createInfo.secondaryInfo.isSecondary) {
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		VkCommandBufferInheritanceInfo inheritenceInfo = {};
		VulkanFramebuffer* framebuffer = static_cast<VulkanFramebuffer *>(createInfo.secondaryInfo.framebuffer);
		VulkanRenderPass* renderPass = static_cast<VulkanRenderPass *>(createInfo.secondaryInfo.renderPass);
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

VulkanCommandBuffer::~VulkanCommandBuffer() {
}

void VulkanCommandBuffer::BeginCommandBuffer() {
	vkResetCommandBuffer(commandBuffer, 0);
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void VulkanCommandBuffer::BindRenderPass(
	RenderPass* renderPass,
	Framebuffer* framebuffer,
	uint32_t width,
	uint32_t height,
	ClearColorValue* colorClearValues,
	uint32_t colorClearCount,
	ClearDepthStencil depthStencilClearValue
) {
	VulkanRenderPass *vulkanRenderPass = static_cast<VulkanRenderPass*>(renderPass);
	VulkanFramebuffer *vulkanFramebuffer = static_cast<VulkanFramebuffer*>(framebuffer);
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

void VulkanCommandBuffer::UnbindRenderPass() {
	vkCmdEndRenderPass(commandBuffer);
	cmdEndDebugUtilsLabelEXT(commandBuffer);
}

void VulkanCommandBuffer::BeginDebugLabelSection(const char* name, float color[4]) {
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

void VulkanCommandBuffer::EndDebugLabelSection() {
	cmdEndDebugUtilsLabelEXT(commandBuffer);
}

void VulkanCommandBuffer::BindGraphicsDescriptorSet(GraphicsPipeline* graphicsPipeline, DescriptorSet** descriptorSets, uint32_t descriptorSetCount) {
	VulkanGraphicsPipeline *vkPipeline = static_cast<VulkanGraphicsPipeline *>(graphicsPipeline);
	BindDescriptorSet(vkPipeline->GetGraphicsPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSets, descriptorSetCount);
}

void VulkanCommandBuffer::BindComputeDescriptorSet(ComputePipeline* graphicsPipeline, DescriptorSet** descriptorSets, uint32_t descriptorSetCount) {
	VulkanComputePipeline* vkPipeline = static_cast<VulkanComputePipeline*>(graphicsPipeline);
	BindDescriptorSet(vkPipeline->GetComputePipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE, descriptorSets, descriptorSetCount);
}

void VulkanCommandBuffer::BindDescriptorSet(VkPipelineLayout pipelineLayout, VkPipelineBindPoint bindPoint, DescriptorSet** descriptorSets, uint32_t descriptorSetCount) {
	std::vector<VkDescriptorSet> vkDescriptorSets;
	vkDescriptorSets.reserve(descriptorSetCount);
	for (uint32_t i = 0; i < descriptorSetCount; i++) {
		auto descriptorPtr = descriptorSets[i];
		VkDescriptorSet desc = descriptorPtr != nullptr
			? static_cast<VulkanDescriptorSet*>(descriptorPtr)->GetDescriptorSet()
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

void VulkanCommandBuffer::BindCommandBuffers(CommandBuffer** commandBuffers, uint32_t commandBuffersCount) {
	std::vector<VkCommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffersCount);
	for (size_t i = 0; i < commandBuffersCount; i++) {
		VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(commandBuffers[i])->GetCommandBuffer();
		vkCommandBuffers.push_back(cmd);
	}

	vkCmdExecuteCommands(
		commandBuffer,
		static_cast<uint32_t>(vkCommandBuffers.size()),
		vkCommandBuffers.data()
	);
}

void VulkanCommandBuffer::SetDepthBias(float biasConstantFactor, float biasSlopeFactor) {
	vkCmdSetDepthBias(
		commandBuffer,
		biasConstantFactor,
		0.0f,
		biasSlopeFactor
	);
}

void VulkanCommandBuffer::SetViewport(float offsetX, float offsetY, float width, float height, float depthMin, float depthMax) {
	VkViewport viewport{};
	viewport.x = offsetX;
	viewport.y = offsetY;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = depthMin;
	viewport.maxDepth = depthMax;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) {
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.width = width;
	scissor.extent.height = height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::BindGraphicsPipeline(GraphicsPipeline* pipeline) {
	VulkanGraphicsPipeline* VulkanGraphicsPipeline = static_cast<GraphicsAPI::VulkanGraphicsPipeline*>(pipeline);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanGraphicsPipeline->GetGraphicsPipeline());
}

void VulkanCommandBuffer::BindComputePipeline(ComputePipeline* pipeline) {
	VulkanComputePipeline *VulkanGraphicsPipeline = static_cast<VulkanComputePipeline*>(pipeline);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VulkanGraphicsPipeline->GetComputePipeline());
}

void VulkanCommandBuffer::BindVertexArrayObject(VertexArrayObject* vertexArrayObject) {
	VulkanVertexArrayObject* vkVao = static_cast<VulkanVertexArrayObject*>(vertexArrayObject);
	auto& vkVertexBuffers = vkVao->GetVertexBuffers();

	BindVertexBuffers(vkVertexBuffers.data(), static_cast<uint32_t>(vkVertexBuffers.size()));
	BindIndexBuffer(vkVao->GetIndexBuffer());
}

void VulkanCommandBuffer::BindVertexBuffers(VertexBuffer** vertexBuffers, uint32_t vertexBufferCount) {
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;
	buffers.resize(vertexBufferCount);
	offsets.resize(vertexBufferCount);

	for (uint32_t i = 0; i < vertexBufferCount; ++i) {
		VulkanVertexBuffer *vb = static_cast<VulkanVertexBuffer *>(vertexBuffers[i]);
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

void VulkanCommandBuffer::BindIndexBuffer(IndexBuffer* indexBuffer) {
	VulkanIndexBuffer *vulkanIndexBuffer = static_cast<VulkanIndexBuffer *>(indexBuffer);
	vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer->GetBuffer(), 0, vulkanIndexBuffer->Is32Bit() ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
}

void VulkanCommandBuffer::DrawVertices(uint32_t vertexCount, uint32_t instanceCount) {
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
}

void VulkanCommandBuffer::DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t instanceCount, int32_t vertexOffset) {
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, 0);
}

void VulkanCommandBuffer::DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
	vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::WaitForComputeMemoryBarrier(GraphicsAPI::RenderTarget* renderTarget, bool shouldMakeWritable) {
	GraphicsAPI::VulkanRenderTarget* vulkanRenderTarget = static_cast<GraphicsAPI::VulkanRenderTarget*>(renderTarget);
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

void VulkanCommandBuffer::EndCommandBuffer() {
	vkEndCommandBuffer(commandBuffer);
}
