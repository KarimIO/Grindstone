#include "DirectX12CommandBuffer.hpp"
#include "DirectX12RenderPass.hpp"
#include "DirectX12GraphicsPipeline.hpp"
#include "DirectX12Framebuffer.hpp"
#include "DirectX12VertexBuffer.hpp"
#include "DirectX12IndexBuffer.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12UniformBuffer.hpp"
#include "DirectX12Texture.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		ID3D12CommandList* DirectX12CommandBuffer::getCommandList() {
			return command_list_;
		}
		void DirectX12CommandBuffer::handleStep(CommandCreateInfo* ci) {
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

		DirectX12CommandBuffer::DirectX12CommandBuffer(CommandBufferCreateInfo ci) {
			auto graphics_wrapper = DirectX12GraphicsWrapper::get();
			auto device = graphics_wrapper.device_;
			auto command_alloc = graphics_wrapper.graphics_command_allocator_;

			HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_alloc, NULL, IID_PPV_ARGS(&command_list_));
			if (FAILED(hr)) {
				throw std::runtime_error("DirectX12: Could not CreateCommandList");
			}

			if (ci.count > 0) {

				for (uint32_t j = 0; j < ci.count; j++) {
					handleStep(ci.steps[j]);
				}
			}

			command_list_->Close();
		}

		DirectX12CommandBuffer::~DirectX12CommandBuffer() {
		}

		void DirectX12CommandBuffer::uploadCmdBindRenderPass(CommandBindRenderPass* ci) {
			//command_list_->clear
			/*DirectX12RenderPass* rp = (DirectX12RenderPass*)(ci->renderPass);
			DirectX12Framebuffer* fb = (DirectX12Framebuffer*)(ci->framebuffer);
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

			vkCmdBeginRenderPass(command_buffer_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);*/
		}

		void DirectX12CommandBuffer::uploadCmdUnbindRenderPass(CommandUnbindRenderPass* ci) {
			//vkCmdEndRenderPass(command_buffer_);
		}

		void DirectX12CommandBuffer::uploadCmdBindDescriptorSet(CommandBindDescriptorSets* ci) {
			/*DirectX12GraphicsPipeline* graphicsPipeline = (DirectX12GraphicsPipeline*)(ci->graphicsPipeline);
			std::vector<VkDescriptorSet> descriptorSets;
			descriptorSets.reserve(ci->uniformBufferCount + ci->textureCount);
			for (uint32_t i = 0; i < ci->uniformBufferCount; i++) {
				DirectX12UniformBuffer* vkub = (DirectX12UniformBuffer*)(ci->uniformBuffers[i]);
				descriptorSets.push_back(vkub->getDescriptorSet());
			}

			for (uint32_t i = 0; i < ci->textureCount; i++) {
				DirectX12TextureBinding* vktex = (DirectX12TextureBinding*)(ci->textureBindings[i]);
				descriptorSets.push_back(vktex->getDescriptorSet());
			}

			vkCmdBindDescriptorSets(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getGraphicsPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);*/
		}

		void DirectX12CommandBuffer::uploadCmdBindCommandBuffers(CommandCallCmdBuffer* ci) {
			/*std::vector<VkCommandBuffer> commandBuffers;
			commandBuffers.reserve(ci->commandBuffersCount);
			for (size_t i = 0; i < ci->commandBuffersCount; i++) {
				VkCommandBuffer cmd = static_cast<DirectX12CommandBuffer*>(ci->commandBuffers[i])->getCommandBuffer();
				commandBuffers.push_back(cmd);
			}

			vkCmdExecuteCommands(command_buffer_, commandBuffers.size(), commandBuffers.data());*/
		}

		void DirectX12CommandBuffer::uploadCmdBindGraphicsPipeline(CommandBindGraphicsPipeline* ci) {
			DirectX12GraphicsPipeline* graphicsPipeline = (DirectX12GraphicsPipeline*)(ci->graphicsPipeline);
			/*command_list_->SetGraphicsRootSignature(rootSignature); // set the root signature
			command_list_->RSSetViewports(1, &viewport); // set the viewports
			command_list_->RSSetScissorRects(1, &scissorRect); // set the scissor rects
			command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);*/
			command_list_->SetPipelineState(graphicsPipeline->getPipelineState());
		}

		void DirectX12CommandBuffer::uploadCmdBindVertexBuffers(CommandBindVBOs* ci) {
			DirectX12VertexBuffer* vertexBuffer = dynamic_cast<DirectX12VertexBuffer*>(ci->vertexBuffer[0]);
			
			//command_list_->IASetVertexBuffers(0, 1, vertexBuffer->getView());
		}

		void DirectX12CommandBuffer::uploadCmdBindIndexBuffer(CommandBindIBO* ci) {
			DirectX12IndexBuffer* indexBuffer = dynamic_cast<DirectX12IndexBuffer*>(ci->indexBuffer);
			//vkCmdBindIndexBuffer(command_buffer_, indexBuffer->getBuffer(), 0, ci->useLargeBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void DirectX12CommandBuffer::uploadCmdDrawVertices(CommandDrawVertices* ci) {
			command_list_->DrawInstanced(ci->count, ci->numInstances, 0, 0);
		}

		void DirectX12CommandBuffer::uploadCmdDrawIndices(CommandDrawIndices* ci) {
			command_list_->DrawIndexedInstanced(ci->count, ci->numInstances, ci->indexStart, ci->baseVertex, 0);
		}
	}
}
