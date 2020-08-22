#include "DirectX11CommandBuffer.hpp"
#include "DirectX11RenderPass.hpp"
#include "DirectX11GraphicsPipeline.hpp"
#include "DirectX11Framebuffer.hpp"
#include "DirectX11VertexBuffer.hpp"
#include "DirectX11IndexBuffer.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11UniformBuffer.hpp"
#include "DirectX11Texture.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		ID3D12CommandList* DirectX11CommandBuffer::getCommandList() {
			return command_list_;
		}

		void DirectX11CommandBuffer::handleStep(CommandCreateInfo* ci) {
			switch (ci->type) {
			case CommandBufferType::CallCommandBuffer: {
				uploadCmdBindCommandBuffers(static_cast<CommandCallCmdBuffer*>(ci));
				return;
			}
			case CommandBufferType::BindRenderPass: {
				uploadCmdBindRenderPass(static_cast<CommandBindRenderPass*>(ci));
				return;
			}
			case CommandBufferType::UnbindRenderPass: {
				uploadCmdUnbindRenderPass(static_cast<CommandUnbindRenderPass*>(ci));
				return;
			}
			case CommandBufferType::BindVertexBuffers: {
				uploadCmdBindVertexBuffers(static_cast<CommandBindVBOs*>(ci));
				return;
			}
			case CommandBufferType::BindIndexBuffer: {
				uploadCmdBindIndexBuffer(static_cast<CommandBindIBO*>(ci));
				return;
			}
			case CommandBufferType::BindGraphicsPipeline: {
				uploadCmdBindGraphicsPipeline(static_cast<CommandBindGraphicsPipeline*>(ci));
				return;
			}
			case CommandBufferType::BindDescriptorSet: {
				uploadCmdBindDescriptorSet(static_cast<CommandBindDescriptorSets*>(ci));
				return;
			}
			case CommandBufferType::DrawVertices: {
				uploadCmdDrawVertices(static_cast<CommandDrawVertices*>(ci));
				return;
			}
			case CommandBufferType::DrawVerticesIndices: {
				uploadCmdDrawIndices(static_cast<CommandDrawIndices*>(ci));
				return;
			}
			}
		}

		DirectX11CommandBuffer::DirectX11CommandBuffer(CommandBufferCreateInfo ci) {
			VkDevice device = DirectX11GraphicsWrapper::get().getDevice();

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = DirectX11GraphicsWrapper::get().getGraphicsCommandPool();
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
					DirectX11Framebuffer* fb = static_cast<DirectX11Framebuffer*>(ci.secondaryInfo.framebuffer);
					DirectX11RenderPass* rp = static_cast<DirectX11RenderPass*>(ci.secondaryInfo.renderPass);
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

		DirectX11CommandBuffer::~DirectX11CommandBuffer() {
		}

		void DirectX11CommandBuffer::uploadCmdBindRenderPass(CommandBindRenderPass* ci) {
			DirectX11RenderPass* rp = (DirectX11RenderPass*)(ci->renderPass);
			DirectX11Framebuffer* fb = (DirectX11Framebuffer*)(ci->framebuffer);
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

		void DirectX11CommandBuffer::uploadCmdUnbindRenderPass(CommandUnbindRenderPass* ci) {
			vkCmdEndRenderPass(command_buffer_);
		}

		void DirectX11CommandBuffer::uploadCmdBindDescriptorSet(CommandBindDescriptorSets* ci) {
			DirectX11GraphicsPipeline* graphicsPipeline = (DirectX11GraphicsPipeline*)(ci->graphicsPipeline);
			std::vector<VkDescriptorSet> descriptorSets;
			descriptorSets.reserve(ci->uniformBufferCount + ci->textureCount);
			for (uint32_t i = 0; i < ci->uniformBufferCount; i++) {
				DirectX11UniformBuffer* vkub = (DirectX11UniformBuffer*)(ci->uniformBuffers[i]);
				descriptorSets.push_back(vkub->getDescriptorSet());
			}

			for (uint32_t i = 0; i < ci->textureCount; i++) {
				DirectX11TextureBinding* vktex = (DirectX11TextureBinding*)(ci->textureBindings[i]);
				descriptorSets.push_back(vktex->getDescriptorSet());
			}

			vkCmdBindDescriptorSets(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getGraphicsPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
		}

		void DirectX11CommandBuffer::uploadCmdBindCommandBuffers(CommandCallCmdBuffer* ci) {
			std::vector<VkCommandBuffer> commandBuffers;
			commandBuffers.reserve(ci->commandBuffersCount);
			for (size_t i = 0; i < ci->commandBuffersCount; i++) {
				VkCommandBuffer cmd = static_cast<DirectX11CommandBuffer*>(ci->commandBuffers[i])->getCommandBuffer();
				commandBuffers.push_back(cmd);
			}

			vkCmdExecuteCommands(command_buffer_, commandBuffers.size(), commandBuffers.data());

		}

		void DirectX11CommandBuffer::uploadCmdBindGraphicsPipeline(CommandBindGraphicsPipeline* ci) {
			DirectX11GraphicsPipeline* graphicsPipeline = (DirectX11GraphicsPipeline*)(ci->graphicsPipeline);
			vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getGraphicsPipeline());
		}

		void DirectX11CommandBuffer::uploadCmdBindVertexBuffers(CommandBindVBOs* ci) {
			DirectX11VertexBuffer* vertexBuffer = dynamic_cast<DirectX11VertexBuffer*>(ci->vertexBuffer[0]);
			VkBuffer vertexBuffers[] = { vertexBuffer->getBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(command_buffer_, 0, 1, vertexBuffers, offsets);
		}

		void DirectX11CommandBuffer::uploadCmdBindIndexBuffer(CommandBindIBO* ci) {
			DirectX11IndexBuffer* indexBuffer = dynamic_cast<DirectX11IndexBuffer*>(ci->indexBuffer);
			vkCmdBindIndexBuffer(command_buffer_, indexBuffer->getBuffer(), 0, ci->useLargeBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void DirectX11CommandBuffer::uploadCmdDrawVertices(CommandDrawVertices* ci) {
			vkCmdDraw(command_buffer_, ci->count, ci->numInstances, 0, 0);
		}

		void DirectX11CommandBuffer::uploadCmdDrawIndices(CommandDrawIndices* ci) {
			vkCmdDrawIndexed(command_buffer_, ci->count, ci->numInstances, ci->indexStart, ci->baseVertex, 0);
		}
	}
}
