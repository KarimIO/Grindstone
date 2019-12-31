#include "VkCommandBuffer.hpp"
#include "VkRenderPass.hpp"
#include "VkGraphicsPipeline.hpp"
#include "VkFramebuffer.hpp"
#include "VkVertexBuffer.hpp"
#include "VkIndexBuffer.hpp"
#include "VkUniformBuffer.hpp"
#include <cstring>

void vkCommandBuffer::HandleStep(uint32_t framebufferID, VkCommandBuffer *commandBuffer, CommandCreateInfo *step) {
	switch (step->type) {
	case CMD_CALL_COMMAND_BUFFER: {
		CommandCallCmdBuffer *createInfo = static_cast<CommandCallCmdBuffer *>(step);

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.reserve(createInfo->commandBuffersCount);
		for (size_t i = 0; i < createInfo->commandBuffersCount; i++) {
			VkCommandBuffer *cmd = static_cast<vkCommandBuffer *>(createInfo->commandBuffers[i])->GetCommandBuffer(framebufferID);
			commandBuffers.push_back(*cmd);
		}

		vkCmdExecuteCommands(*commandBuffer, commandBuffers.size(), commandBuffers.data());

		return;
	}
	case CMD_BIND_RENDER_PASS: {
			CommandBindRenderPass *createInfo = static_cast<CommandBindRenderPass *>(step);
			vkRenderPass *rp = (vkRenderPass *)(createInfo->renderPass);
			vkFramebuffer *fb = (vkFramebuffer *)(createInfo->framebuffers[framebufferID]);
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = *rp->GetRenderPass();
			renderPassInfo.framebuffer = *fb->GetFramebuffer();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { createInfo->width, createInfo->height };

			std::vector<VkClearValue> clearColor;
			clearColor.resize(createInfo->colorClearCount + (createInfo->depthStencilClearValue.hasDepthStencilAttachment?1:0));
			for (size_t i = 0; i < createInfo->colorClearCount; i++) {
				std::memcpy(&clearColor[i].color, &createInfo->colorClearValues[i], sizeof(VkClearColorValue));
			}
			if (createInfo->depthStencilClearValue.hasDepthStencilAttachment)
				clearColor[createInfo->colorClearCount].depthStencil = { createInfo->depthStencilClearValue.depth, createInfo->depthStencilClearValue.stencil };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
			renderPassInfo.pClearValues = clearColor.data();

			vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			return;
		}
	case CMD_UNBIND_RENDER_PASS: {
		vkCmdEndRenderPass(*commandBuffer);
		return;
	}
	case CMD_BIND_VERTEX_BUFFER: {
			CommandBindVBO *createInfo = static_cast<CommandBindVBO *>(step);
			vkVertexBuffer *vertexBuffer = dynamic_cast<vkVertexBuffer *>(createInfo->vertexBuffer);
			VkBuffer vertexBuffers[] = { *vertexBuffer->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
			return;
		}
	case CMD_BIND_INDEX_BUFFER: {
			CommandBindIBO *createInfo = static_cast<CommandBindIBO *>(step);
			vkIndexBuffer *indexBuffer = dynamic_cast<vkIndexBuffer *>(createInfo->indexBuffer);
			vkCmdBindIndexBuffer(*commandBuffer, *indexBuffer->GetBuffer(), 0, createInfo->useLargeBuffer? VK_INDEX_TYPE_UINT32:VK_INDEX_TYPE_UINT16);
			return;
		}
	case CMD_BIND_GRAPHICS_PIPELINE: {
			CommandBindGraphicsPipeline *createInfo = static_cast<CommandBindGraphicsPipeline *>(step);
			vkGraphicsPipeline *graphicsPipeline = (vkGraphicsPipeline *)(createInfo->graphicsPipeline);
			vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline->GetGraphicsPipeline());
			return;
		}
	case CMD_BIND_DESCRIPTOR_SET: {
			CommandBindDescriptorSets *createInfo = static_cast<CommandBindDescriptorSets *>(step);
			vkGraphicsPipeline *graphicsPipeline = (vkGraphicsPipeline *)(createInfo->graphicsPipeline);
			std::vector<VkDescriptorSet> descriptorSets;
			descriptorSets.reserve(createInfo->uniformBufferCount); //  + createInfo->textureCount
			for (uint32_t i = 0; i < createInfo->uniformBufferCount; i++) {
				vkUniformBuffer *vkub = (vkUniformBuffer *)(createInfo->uniformBuffers[i]);
				descriptorSets.push_back(*vkub->GetDescriptorSet());
			}

			/*for (uint32_t i = 0; i < createInfo->textureCount; i++) {
				vkTexture *vktex = (vkTexture *)(createInfo->textures[i]);
				descriptorSets.push_back(*vktex->GetDescriptorSet());
			}*/
			
			vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline->GetPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
			return;
		}
	case CMD_DRAW_VERTEX: {
			CommandDrawVertices *createInfo = static_cast<CommandDrawVertices *>(step);
			vkCmdDraw(*commandBuffer, createInfo->count, createInfo->numInstances, 0, 0);
			return;
		}
	case CMD_DRAW_INDEX: {
			CommandDrawIndexed *createInfo = static_cast<CommandDrawIndexed *>(step);
			vkCmdDrawIndexed(*commandBuffer, createInfo->count, createInfo->numInstances, createInfo->indexStart, createInfo->baseVertex, 0);
			return;
		}
	}
}

vkCommandBuffer::vkCommandBuffer(VkDevice *dev, VkCommandPool *commandPool, CommandBufferCreateInfo createInfo) {
	device = dev;
	commandBuffers.resize(createInfo.numOutputs);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = *commandPool;
	allocInfo.level = createInfo.secondaryInfo.isSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(*device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	secondaryInfo = createInfo.secondaryInfo;

	m_currentBuffer = 0;
	if (createInfo.count == 0) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(commandBuffers.size()); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			if (createInfo.secondaryInfo.isSecondary) {
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				VkCommandBufferInheritanceInfo inheritenceInfo = {};
				vkFramebuffer *fb = static_cast<vkFramebuffer *>(createInfo.secondaryInfo.fbos[i]);
				vkRenderPass *rp = static_cast<vkRenderPass *>(createInfo.secondaryInfo.renderPass);
				inheritenceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritenceInfo.framebuffer = *fb->GetFramebuffer();
				inheritenceInfo.renderPass = *rp->GetRenderPass();
				inheritenceInfo.occlusionQueryEnable = VK_FALSE;
				inheritenceInfo.pipelineStatistics = 0;
				inheritenceInfo.pNext = nullptr;
				inheritenceInfo.subpass = 0;
				beginInfo.pInheritanceInfo = &inheritenceInfo;
			}

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			for (uint32_t j = 0; j < createInfo.count; j++) {
				HandleStep(i, &commandBuffers[i], createInfo.steps[j]);
			}

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}
}

VkCommandBuffer * vkCommandBuffer::GetCommandBuffer(size_t id) {
	return &commandBuffers[id];
}


void vkCommandBuffer::SetCommandBuffer(uint16_t currentBuffer) {
	m_currentBuffer = currentBuffer;
}

void vkCommandBuffer::Reset() {
	VkCommandBufferResetFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkResetCommandBuffer(commandBuffers[m_currentBuffer], flags);
}

void vkCommandBuffer::Begin() {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (secondaryInfo.isSecondary) {
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		VkCommandBufferInheritanceInfo inheritenceInfo = {};
		vkFramebuffer *fb = static_cast<vkFramebuffer *>(secondaryInfo.fbos[m_currentBuffer]);
		vkRenderPass *rp = static_cast<vkRenderPass *>(secondaryInfo.renderPass);
		inheritenceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritenceInfo.framebuffer = *fb->GetFramebuffer();
		inheritenceInfo.renderPass = *rp->GetRenderPass();
		inheritenceInfo.occlusionQueryEnable = VK_FALSE;
		inheritenceInfo.pipelineStatistics = 0;
		inheritenceInfo.pNext = nullptr;
		inheritenceInfo.subpass = 0;
		beginInfo.pInheritanceInfo = &inheritenceInfo;
	}

	vkBeginCommandBuffer(commandBuffers[m_currentBuffer], &beginInfo);
}

void vkCommandBuffer::End() {
	if (vkEndCommandBuffer(commandBuffers[m_currentBuffer]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void vkCommandBuffer::BindRenderPass(RenderPass * renderpass, Framebuffer **framebuffers, uint32_t framebufferCount) {
	vkRenderPass *rp = (vkRenderPass *)(renderpass);
	vkFramebuffer *fb = (vkFramebuffer *)(framebuffers[m_currentBuffer]);
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = *rp->GetRenderPass();
	renderPassInfo.framebuffer = *fb->GetFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { rp->m_width, rp->m_height };

	std::vector<VkClearValue> clearColor;
	clearColor.resize(rp->m_colorClearCount + (rp->m_depthStencilClearValue.hasDepthStencilAttachment ? 1 : 0));
	for (size_t i = 0; i < rp->m_colorClearCount; i++) {
		std::memcpy(&clearColor[i].color, &rp->m_colorClearValues[i], sizeof(VkClearColorValue));
	}
	if (rp->m_depthStencilClearValue.hasDepthStencilAttachment)
		clearColor[rp->m_colorClearCount].depthStencil = { rp->m_depthStencilClearValue.depth, rp->m_depthStencilClearValue.stencil };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();

	vkCmdBeginRenderPass(commandBuffers[m_currentBuffer], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void vkCommandBuffer::UnbindRenderPass() {
	vkCmdEndRenderPass(commandBuffers[m_currentBuffer]);
}

void vkCommandBuffer::BindCommandBuffers(CommandBuffer **command, uint32_t numCommands) {
	commandBuffers.reserve(numCommands);
	for (size_t i = 0; i < numCommands; i++) {
		vkCommandBuffer *cmd = static_cast<vkCommandBuffer *>(command[i]);
		commandBuffers.push_back(*cmd->GetCommandBuffer(0));
	}

	vkCmdExecuteCommands(commandBuffers[m_currentBuffer], commandBuffers.size(), commandBuffers.data());

}

void vkCommandBuffer::BindGraphicsPipeline(GraphicsPipeline * pipeline) {
	vkGraphicsPipeline *graphicsPipeline = (vkGraphicsPipeline *)(pipeline);
	vkCmdBindPipeline(commandBuffers[m_currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline->GetGraphicsPipeline());
}

void vkCommandBuffer::BindTextureDescriptor(TextureBinding * binding) {
}

void vkCommandBuffer::BindUBODescriptor(UniformBufferBinding * binding) {
	//((vkUniformBufferBinding *)binding)->getLayout();
	//vkCmdBindDescriptorSets(commandBuffers[m_currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline->GetPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
}

void vkCommandBuffer::BindBufferObjects(VertexBuffer * vb, IndexBuffer * ib, bool useLargeBuffer) {
	vkVertexBuffer *vertexBuffer = dynamic_cast<vkVertexBuffer *>(vb);
	VkBuffer vertexBuffers[] = { *vertexBuffer->GetBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffers[m_currentBuffer], 0, 1, vertexBuffers, offsets);
	
	vkIndexBuffer *indexBuffer = dynamic_cast<vkIndexBuffer *>(ib);
	vkCmdBindIndexBuffer(commandBuffers[m_currentBuffer], *indexBuffer->GetBuffer(), 0, useLargeBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
	return;
}

void vkCommandBuffer::DrawIndexed(int32_t baseVertex, uint32_t indexStart, uint32_t count, uint32_t numInstances) {
	vkCmdDrawIndexed(commandBuffers[m_currentBuffer], count, numInstances, indexStart, baseVertex, 0);
}
