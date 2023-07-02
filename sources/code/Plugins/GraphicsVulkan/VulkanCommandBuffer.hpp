#pragma once

#include <Common/Graphics/CommandBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanCommandBuffer : public CommandBuffer {
		public:
			VulkanCommandBuffer(CommandBuffer::CreateInfo& createInfo);
			virtual ~VulkanCommandBuffer() override;
		public:
			VkCommandBuffer GetCommandBuffer();
		private:
			virtual void BeginCommandBuffer() override;
			virtual void BindRenderPass(
				RenderPass* renderPass,
				Framebuffer* framebuffer,
				uint32_t width,
				uint32_t height,
				ClearColorValue* colorClearValues,
				uint32_t colorClearCount,
				ClearDepthStencil depthStencilClearValue
			) override;
			virtual void UnbindRenderPass() override;
			virtual void BindDescriptorSet(
				Pipeline* graphicsPipeline,
				DescriptorSet** descriptorSets,
				uint32_t descriptorSetCount
			) override;
			virtual void BindCommandBuffers(CommandBuffer** commandBuffers, uint32_t commandBuffersCount) override;
			virtual void BindPipeline(Pipeline* pipeline) override;
			virtual void BindVertexBuffers(VertexBuffer** vb, uint32_t count) override;
			virtual void BindIndexBuffer(IndexBuffer* indexBuffer, bool useLargeBuffer) override;
			virtual void DrawVertices(uint32_t vertexCount, uint32_t instanceCount) override;
			virtual void DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t instanceCount, int32_t vertexOffset) override;
			virtual void EndCommandBuffer() override;
		private:
			VkCommandBuffer commandBuffer;
			VkCommandBufferBeginInfo beginInfo;
			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};
