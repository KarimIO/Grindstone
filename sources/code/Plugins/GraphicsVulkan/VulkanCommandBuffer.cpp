#include "VulkanCommandBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanIndexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		void VulkanCommandBuffer::HandleStep(Command* ci) {
			switch (ci->type) {
			case CommandBufferType::CallCommandBuffer: {
				UploadCmdBindCommandBuffers(static_cast<CommandCallCmdBuffer *>(ci));
				return;
			}
			case CommandBufferType::BindRenderPass: {
				UploadCmdBindRenderPass(static_cast<CommandBindRenderPass *>(ci));
				return;
			}
			case CommandBufferType::UnbindRenderPass: {
				UploadCmdUnbindRenderPass(static_cast<CommandUnbindRenderPass *>(ci));
				return;
			}
			case CommandBufferType::BindVertexBuffers: {
				UploadCmdBindVertexBuffers(static_cast<CommandBindVBOs *>(ci));
				return;
			}
			case CommandBufferType::BindIndexBuffer: {
				UploadCmdBindIndexBuffer(static_cast<CommandBindIBO *>(ci));
				return;
			}
			case CommandBufferType::BindPipeline: {
				UploadCmdBindPipeline(static_cast<CommandBindPipeline *>(ci));
				return;
			}
			case CommandBufferType::BindDescriptorSet: {
				UploadCmdBindDescriptorSet(static_cast<CommandBindDescriptorSets *>(ci));
				return;
			}
			case CommandBufferType::DrawVertices: {
				UploadCmdDrawVertices(static_cast<CommandDrawVertices *>(ci));
				return;
			}
			case CommandBufferType::DrawVerticesIndices: {
				UploadCmdDrawIndices(static_cast<CommandDrawIndices *>(ci));
				return;
			}
			}
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

			secondaryInfo = createInfo.secondaryInfo;

			if (createInfo.count > 0) {
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

				if (createInfo.secondaryInfo.isSecondary) {
					beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
					VkCommandBufferInheritanceInfo inheritenceInfo = {};
					VulkanFramebuffer * framebuffer = static_cast<VulkanFramebuffer *>(createInfo.secondaryInfo.framebuffer);
					VulkanRenderPass *renderPass = static_cast<VulkanRenderPass *>(createInfo.secondaryInfo.renderPass);
					inheritenceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
					inheritenceInfo.framebuffer = framebuffer->GetFramebuffer();
					inheritenceInfo.renderPass = renderPass->GetRenderPassHandle();
					inheritenceInfo.occlusionQueryEnable = VK_FALSE;
					inheritenceInfo.pipelineStatistics = 0;
					inheritenceInfo.pNext = nullptr;
					inheritenceInfo.subpass = 0;
					beginInfo.pInheritanceInfo = &inheritenceInfo;
				}

				vkBeginCommandBuffer(commandBuffer, &beginInfo);

				for (uint32_t j = 0; j < createInfo.count; j++) {
					HandleStep(createInfo.steps[j]);
				}

				if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
					throw std::runtime_error("failed to record command buffer!");
				}
			}
		}

		VulkanCommandBuffer::~VulkanCommandBuffer() {
		}

		void VulkanCommandBuffer::UploadCmdBindRenderPass(CommandBindRenderPass * createInfo) {
			VulkanRenderPass *renderPass = (VulkanRenderPass *)(createInfo->renderPass);
			VulkanFramebuffer *framebuffer = (VulkanFramebuffer *)(createInfo->framebuffer);
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass->GetRenderPassHandle();
			renderPassInfo.framebuffer = framebuffer->GetFramebuffer();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { createInfo->width, createInfo->height };

			size_t clearColorCount = createInfo->colorClearCount;
			clearColorCount += createInfo->depthStencilClearValue.hasDepthStencilAttachment ? 1 : 0;

			std::vector<VkClearValue> clearColor;
			clearColor.resize(clearColorCount);
			for (size_t i = 0; i < createInfo->colorClearCount; i++) {
				std::memcpy(&clearColor[i].color, &createInfo->colorClearValues[i], sizeof(VkClearColorValue));
			}

			if (createInfo->depthStencilClearValue.hasDepthStencilAttachment) {
				clearColor[createInfo->colorClearCount].depthStencil = {
					createInfo->depthStencilClearValue.depth,
					createInfo->depthStencilClearValue.stencil
				};
			}

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
			renderPassInfo.pClearValues = clearColor.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		void VulkanCommandBuffer::UploadCmdUnbindRenderPass(CommandUnbindRenderPass * createInfo) {
			vkCmdEndRenderPass(commandBuffer);
		}

		void VulkanCommandBuffer::UploadCmdBindDescriptorSet(CommandBindDescriptorSets * createInfo) {
			VulkanPipeline *graphicsPipeline = static_cast<VulkanPipeline *>(createInfo->graphicsPipeline);
			size_t descriptorSetCount = static_cast<size_t>(createInfo->descriptorSetCount);

			std::vector<VkDescriptorSet> descriptorSets;
			descriptorSets.reserve(descriptorSetCount);
			for (uint32_t i = 0; i < createInfo->descriptorSetCount; i++) {
				VulkanDescriptorSet* vkub = (VulkanDescriptorSet *)(createInfo->descriptorSets[i]);
				descriptorSets.push_back(vkub->GetDescriptorSet());
			}

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->GetGraphicsPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
		}

		void VulkanCommandBuffer::UploadCmdBindCommandBuffers(CommandCallCmdBuffer * createInfo) {
			std::vector<VkCommandBuffer> commandBuffers;
			commandBuffers.reserve(createInfo->commandBuffersCount);
			for (size_t i = 0; i < createInfo->commandBuffersCount; i++) {
				VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(createInfo->commandBuffers[i])->GetCommandBuffer();
				commandBuffers.push_back(cmd);
			}

			vkCmdExecuteCommands(commandBuffer, (uint32_t)commandBuffers.size(), commandBuffers.data());

		}

		void VulkanCommandBuffer::UploadCmdBindPipeline(CommandBindPipeline * ci) {
			VulkanPipeline *pipeline = (VulkanPipeline *)(ci->graphicsPipeline);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetGraphicsPipeline());
		}

		void VulkanCommandBuffer::UploadCmdBindVertexBuffers(CommandBindVBOs* ci) {
			VulkanVertexBuffer *vertexBuffer = static_cast<VulkanVertexBuffer *>(ci->vertexBuffer[0]);
			VkBuffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		}

		void VulkanCommandBuffer::UploadCmdBindIndexBuffer(CommandBindIBO * ci) {
			VulkanIndexBuffer *indexBuffer = static_cast<VulkanIndexBuffer *>(ci->indexBuffer);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, ci->useLargeBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void VulkanCommandBuffer::UploadCmdDrawVertices(CommandDrawVertices * ci) {
			vkCmdDraw(commandBuffer, ci->count, ci->numInstances, 0, 0);
		}

		void VulkanCommandBuffer::UploadCmdDrawIndices(CommandDrawIndices * ci){
			vkCmdDrawIndexed(commandBuffer, ci->count, ci->numInstances, ci->indexStart, ci->baseVertex, 0);
		}
	}
}
