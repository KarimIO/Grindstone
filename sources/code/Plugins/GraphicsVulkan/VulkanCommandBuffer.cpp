#include "VulkanCommandBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanIndexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		void VulkanCommandBuffer::handleStep(Command* ci) {
			switch (ci->type) {
			case CommandBufferType::CallCommandBuffer: {
				uploadCmdBindCommandBuffers(static_cast<CommandCallCmdBuffer *>(ci));
				return;
			}
			case CommandBufferType::BindRenderPass: {
				uploadCmdBindRenderPass(static_cast<CommandBindRenderPass *>(ci));
				return;
			}
			case CommandBufferType::UnbindRenderPass: {
				uploadCmdUnbindRenderPass(static_cast<CommandUnbindRenderPass *>(ci));
				return;
			}
			case CommandBufferType::BindVertexBuffers: {
				uploadCmdBindVertexBuffers(static_cast<CommandBindVBOs *>(ci));
				return;
			}
			case CommandBufferType::BindIndexBuffer: {
				uploadCmdBindIndexBuffer(static_cast<CommandBindIBO *>(ci));
				return;
			}
			case CommandBufferType::BindPipeline: {
				uploadCmdBindPipeline(static_cast<CommandBindPipeline *>(ci));
				return;
			}
			case CommandBufferType::BindDescriptorSet: {
				uploadCmdBindDescriptorSet(static_cast<CommandBindDescriptorSets *>(ci));
				return;
			}
			case CommandBufferType::DrawVertices: {
				uploadCmdDrawVertices(static_cast<CommandDrawVertices *>(ci));
				return;
			}
			case CommandBufferType::DrawVerticesIndices: {
				uploadCmdDrawIndices(static_cast<CommandDrawIndices *>(ci));
				return;
			}
			}
		}

		VkCommandBuffer VulkanCommandBuffer::getCommandBuffer()	{
			return command_buffer_;
		}

		VulkanCommandBuffer::VulkanCommandBuffer(CommandBuffer::CreateInfo& ci) {
			VkDevice device = VulkanCore::get().getDevice();

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = VulkanCore::get().getGraphicsCommandPool();
			allocInfo.level = ci.secondaryInfo.isSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			if (vkAllocateCommandBuffers(device, &allocInfo, &command_buffer_) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}

			secondaryInfo = ci.secondaryInfo;

			if (ci.count > 0) {
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

				if (ci.secondaryInfo.isSecondary) {
					beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
					VkCommandBufferInheritanceInfo inheritenceInfo = {};
					VulkanFramebuffer *fb = static_cast<VulkanFramebuffer *>(ci.secondaryInfo.framebuffer);
					VulkanRenderPass *rp = static_cast<VulkanRenderPass *>(ci.secondaryInfo.renderPass);
					inheritenceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
					inheritenceInfo.framebuffer = fb->getFramebuffer();
					inheritenceInfo.renderPass = rp->getRenderPassHandle();
					inheritenceInfo.occlusionQueryEnable = VK_FALSE;
					inheritenceInfo.pipelineStatistics = 0;
					inheritenceInfo.pNext = nullptr;
					inheritenceInfo.subpass = 0;
					beginInfo.pInheritanceInfo = &inheritenceInfo;
				}

				vkBeginCommandBuffer(command_buffer_, &beginInfo);

				for (uint32_t j = 0; j < ci.count; j++) {
					handleStep(ci.steps[j]);
				}

				if (vkEndCommandBuffer(command_buffer_) != VK_SUCCESS) {
					throw std::runtime_error("failed to record command buffer!");
				}
			}
		}

		VulkanCommandBuffer::~VulkanCommandBuffer() {
		}

		void VulkanCommandBuffer::uploadCmdBindRenderPass(CommandBindRenderPass * ci) {
			VulkanRenderPass *rp = (VulkanRenderPass *)(ci->renderPass);
			VulkanFramebuffer *fb = (VulkanFramebuffer *)(ci->framebuffer);
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = rp->getRenderPassHandle();
			renderPassInfo.framebuffer = fb->getFramebuffer();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { ci->width, ci->height };

			std::vector<VkClearValue> clearColor;
			clearColor.resize(ci->colorClearCount + (ci->depthStencilClearValue.hasDepthStencilAttachment ? 1 : 0));
			for (size_t i = 0; i < ci->colorClearCount; i++) {
				std::memcpy(&clearColor[i].color, &ci->colorClearValues[i], sizeof(VkClearColorValue));
			}
			if (ci->depthStencilClearValue.hasDepthStencilAttachment)
				clearColor[ci->colorClearCount].depthStencil = { ci->depthStencilClearValue.depth, ci->depthStencilClearValue.stencil };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
			renderPassInfo.pClearValues = clearColor.data();

			vkCmdBeginRenderPass(command_buffer_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		void VulkanCommandBuffer::uploadCmdUnbindRenderPass(CommandUnbindRenderPass * ci) {
			vkCmdEndRenderPass(command_buffer_);
		}

		void VulkanCommandBuffer::uploadCmdBindDescriptorSet(CommandBindDescriptorSets *ci) {
			VulkanPipeline *graphicsPipeline = (VulkanPipeline *)(ci->graphicsPipeline);
			std::vector<VkDescriptorSet> descriptorSets;
			descriptorSets.reserve(ci->uniformBufferCount + ci->textureCount);
			for (uint32_t i = 0; i < ci->uniformBufferCount; i++) {
				VulkanUniformBuffer *vkub = (VulkanUniformBuffer *)(ci->uniformBuffers[i]);
				descriptorSets.push_back(vkub->getDescriptorSet());
			}

			for (uint32_t i = 0; i < ci->textureCount; i++) {
				VulkanTextureBinding *vktex = (VulkanTextureBinding *)(ci->textureBindings[i]);
				descriptorSets.push_back(vktex->getDescriptorSet());
			}

			vkCmdBindDescriptorSets(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getGraphicsPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
		}

		void VulkanCommandBuffer::uploadCmdBindCommandBuffers(CommandCallCmdBuffer * ci) {
			std::vector<VkCommandBuffer> commandBuffers;
			commandBuffers.reserve(ci->commandBuffersCount);
			for (size_t i = 0; i < ci->commandBuffersCount; i++) {
				VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(ci->commandBuffers[i])->getCommandBuffer();
				commandBuffers.push_back(cmd);
			}

			vkCmdExecuteCommands(command_buffer_, (uint32_t)commandBuffers.size(), commandBuffers.data());

		}

		void VulkanCommandBuffer::uploadCmdBindPipeline(CommandBindPipeline * ci) {
			VulkanPipeline *pipeline = (VulkanPipeline *)(ci->graphicsPipeline);
			vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
		}

		void VulkanCommandBuffer::uploadCmdBindVertexBuffers(CommandBindVBOs* ci) {
			VulkanVertexBuffer *vertexBuffer = dynamic_cast<VulkanVertexBuffer *>(ci->vertexBuffer[0]);
			VkBuffer vertexBuffers[] = { vertexBuffer->getBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(command_buffer_, 0, 1, vertexBuffers, offsets);
		}

		void VulkanCommandBuffer::uploadCmdBindIndexBuffer(CommandBindIBO * ci) {
			VulkanIndexBuffer *indexBuffer = dynamic_cast<VulkanIndexBuffer *>(ci->indexBuffer);
			vkCmdBindIndexBuffer(command_buffer_, indexBuffer->getBuffer(), 0, ci->useLargeBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void VulkanCommandBuffer::uploadCmdDrawVertices(CommandDrawVertices * ci) {
			vkCmdDraw(command_buffer_, ci->count, ci->numInstances, 0, 0);
		}

		void VulkanCommandBuffer::uploadCmdDrawIndices(CommandDrawIndices * ci){
			vkCmdDrawIndexed(command_buffer_, ci->count, ci->numInstances, ci->indexStart, ci->baseVertex, 0);
		}
	}
}
