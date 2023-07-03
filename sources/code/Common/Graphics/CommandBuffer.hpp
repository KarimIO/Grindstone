#pragma once

#include <vector>
#include "Framebuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
		class Framebuffer;
		class DescriptorSet;
		class Pipeline;
		class VertexArrayObject;
		class VertexBuffer;
		class IndexBuffer;

		class CommandBuffer {
		public:
			virtual ~CommandBuffer() {};

			virtual void BeginCommandBuffer() = 0;
			virtual void BindRenderPass(
				RenderPass* renderPass,
				Framebuffer* framebuffer,
				uint32_t width,
				uint32_t height,
				ClearColorValue* colorClearValues,
				uint32_t colorClearCount,
				ClearDepthStencil depthStencilClearValue
			) = 0;
			virtual void UnbindRenderPass() = 0;
			virtual void BindDescriptorSet(
				Pipeline* graphicsPipeline,
				DescriptorSet** descriptorSets,
				uint32_t descriptorSetCount
			) = 0;
			virtual void BindCommandBuffers(CommandBuffer** commandBuffers, uint32_t commandBuffersCount) = 0;
			virtual void BindPipeline(Pipeline* pipeline) = 0;
			virtual void BindVertexArrayObject(VertexArrayObject* vertexArrayObject) = 0;
			virtual void BindVertexBuffers(VertexBuffer** vb, uint32_t count) = 0;
			virtual void BindIndexBuffer(IndexBuffer* indexBuffer) = 0;
			virtual void DrawVertices(uint32_t vertexCount, uint32_t instanceCount) = 0;
			virtual void DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t instanceCount, int32_t vertexOffset) = 0;
			virtual void EndCommandBuffer() = 0;

			struct CommandBufferSecondaryInfo {
				bool isSecondary;
				Framebuffer *framebuffer;
				RenderPass *renderPass;
			};

			struct CreateInfo {
				CommandBufferSecondaryInfo secondaryInfo;
			};
		};
	};
};
