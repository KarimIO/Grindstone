#pragma once

#include <Common/Graphics/CommandBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class CommandBuffer : public Grindstone::GraphicsAPI::CommandBuffer {
	public:
		static void SetupDebugLabelUtils(VkInstance instance);

		CommandBuffer(const CreateInfo& createInfo);
		virtual ~CommandBuffer() override;
	public:
		virtual VkCommandBuffer GetCommandBuffer();
	private:
		virtual void BeginCommandBuffer() override;
		virtual void BindRenderPass(
			Grindstone::GraphicsAPI::RenderPass* renderPass,
			Grindstone::GraphicsAPI::Framebuffer* framebuffer,
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
			GraphicsAPI::GraphicsPipeline* graphicsPipeline,
			GraphicsAPI::DescriptorSet** descriptorSets,
			uint32_t descriptorSetCount
		) override;
		virtual void BindComputeDescriptorSet(
			GraphicsAPI::ComputePipeline* graphicsPipeline,
			GraphicsAPI::DescriptorSet** descriptorSets,
			uint32_t descriptorSetCount
		) override;
		virtual void BindCommandBuffers(Grindstone::GraphicsAPI::CommandBuffer** commandBuffers, uint32_t commandBuffersCount) override;
		virtual void SetViewport(float offsetX, float offsetY, float width, float height, float depthMin = 0.0f, float depthMax = 1.0f) override;
		virtual void SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) override;
		virtual void SetDepthBias(float biasConstantFactor, float biasSlopeFactor) override;
		virtual void BindGraphicsPipeline(GraphicsAPI::GraphicsPipeline* pipeline) override;
		virtual void BindComputePipeline(GraphicsAPI::ComputePipeline* pipeline) override;
		virtual void BindVertexArrayObject(GraphicsAPI::VertexArrayObject* vertexArrayObject) override;
		virtual void BindVertexBuffers(GraphicsAPI::Buffer** vb, uint32_t count) override;
		virtual void BindIndexBuffer(GraphicsAPI::Buffer* indexBuffer) override;
		virtual void DrawVertices(uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) override;
		virtual void DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) override;
		virtual void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
		virtual void BlitDepthImage(Grindstone::GraphicsAPI::DepthStencilTarget* src, Grindstone::GraphicsAPI::DepthStencilTarget* dst) override;
		virtual void WaitForComputeMemoryBarrier(Grindstone::GraphicsAPI::RenderTarget* renderTarget, bool shouldMakeWritable) override;

		virtual void EndCommandBuffer() override;
	private:
		virtual void BindDescriptorSet(
			VkPipelineLayout pipelineLayout,
			VkPipelineBindPoint bindPoint,
			GraphicsAPI::DescriptorSet** descriptorSets,
			uint32_t descriptorSetCount
		);

		VkCommandBuffer commandBuffer;
		VkCommandBufferBeginInfo beginInfo;
		CommandBufferSecondaryInfo secondaryInfo;
	};
}
