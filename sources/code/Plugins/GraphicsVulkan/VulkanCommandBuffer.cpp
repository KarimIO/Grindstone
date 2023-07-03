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

			beginInfo = {};
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
			VkRenderPassBeginInfo renderPassInfo = {};
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

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		void VulkanCommandBuffer::UnbindRenderPass() {
			vkCmdEndRenderPass(commandBuffer);
		}

		void VulkanCommandBuffer::BindDescriptorSet(Pipeline* graphicsPipeline, DescriptorSet** descriptorSets, uint32_t descriptorSetCount) {
			VulkanPipeline *vkPipeline = static_cast<VulkanPipeline *>(graphicsPipeline);

			std::vector<VkDescriptorSet> vkDescriptorSets;
			vkDescriptorSets.reserve(descriptorSetCount);
			for (uint32_t i = 0; i < descriptorSetCount; i++) {
				VulkanDescriptorSet* desc = static_cast<VulkanDescriptorSet*>(descriptorSets[i]);
				vkDescriptorSets.push_back(desc->GetDescriptorSet());
			}

			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vkPipeline->GetGraphicsPipelineLayout(),
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

		void VulkanCommandBuffer::BindPipeline(Pipeline* pipeline) {
			VulkanPipeline *vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetGraphicsPipeline());
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

		void VulkanCommandBuffer::BindIndexBuffer(IndexBuffer* indexBuffer, bool useLargeBuffer) {
			VulkanIndexBuffer *vulkanIndexBuffer = static_cast<VulkanIndexBuffer *>(indexBuffer);
			vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer->GetBuffer(), 0, useLargeBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void VulkanCommandBuffer::DrawVertices(uint32_t vertexCount, uint32_t instanceCount) {
			vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
		}

		void VulkanCommandBuffer::DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t instanceCount, int32_t vertexOffset) {
			vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, 0);
		}

		void VulkanCommandBuffer::EndCommandBuffer() {
			vkEndCommandBuffer(commandBuffer);
		}
	}
}
