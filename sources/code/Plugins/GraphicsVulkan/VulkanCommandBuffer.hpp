#pragma once

#include <Common/Graphics/CommandBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanCommandBuffer : public CommandBuffer {
		public:
			static void SetupDebugLabelUtils(VkInstance instance);

			VulkanCommandBuffer(CommandBuffer::CreateInfo& createInfo);
			virtual ~VulkanCommandBuffer() override;
		public:
			virtual VkCommandBuffer GetCommandBuffer();
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
			virtual void BeginDebugLabelSection(const char* name, float color[4] = nullptr) override;
			virtual void EndDebugLabelSection() override;
			virtual void BindGraphicsDescriptorSet(
				GraphicsPipeline* graphicsPipeline,
				DescriptorSet** descriptorSets,
				uint32_t descriptorSetCount
			) override;
			virtual void BindComputeDescriptorSet(
				ComputePipeline* graphicsPipeline,
				DescriptorSet** descriptorSets,
				uint32_t descriptorSetCount
			) override;
			virtual void BindCommandBuffers(CommandBuffer** commandBuffers, uint32_t commandBuffersCount) override;
			virtual void SetViewport(float offsetX, float offsetY, float width, float height, float depthMin = 0.0f, float depthMax = 1.0f) override;
			virtual void SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) override;
			virtual void SetDepthBias(float biasConstantFactor, float biasSlopeFactor) override;
			virtual void BindGraphicsPipeline(GraphicsPipeline* pipeline) override;
			virtual void BindComputePipeline(ComputePipeline* pipeline) override;
			virtual void BindVertexArrayObject(VertexArrayObject* vertexArrayObject) override;
			virtual void BindVertexBuffers(VertexBuffer** vb, uint32_t count) override;
			virtual void BindIndexBuffer(IndexBuffer* indexBuffer) override;
			virtual void DrawVertices(uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) override;
			virtual void DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) override;
			virtual void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

			virtual void WaitForComputeMemoryBarrier(RenderTarget* renderTarget, bool shouldMakeWritable) override;

			virtual void EndCommandBuffer() override;
		private:
			virtual void BindDescriptorSet(
				VkPipelineLayout pipelineLayout,
				VkPipelineBindPoint bindPoint,
				DescriptorSet** descriptorSets,
				uint32_t descriptorSetCount
			);

			VkCommandBuffer commandBuffer;
			VkCommandBufferBeginInfo beginInfo;
			CommandBufferSecondaryInfo secondaryInfo;
		};
	};
};
