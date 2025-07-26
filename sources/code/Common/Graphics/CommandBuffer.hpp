#pragma once

#include <vector>
#include "Framebuffer.hpp"

namespace Grindstone::GraphicsAPI {
	class Buffer;
	class RenderPass;
	class Framebuffer;
	class DescriptorSet;
	class GraphicsPipeline;
	class ComputePipeline;
	class VertexArrayObject;
	class DepthStencilTarget;

	enum class PipelineStage : uint8_t {
		TopOfPipe,
		FragmentShader,
		ComputeShader,
		ColorAttachmentOutput,
		BottomOfPipe,
	};

	enum class AccessFlags : uint8_t {
		None = 0,
		Read = 1 << 0,
		Write = 1 << 1,
	};

	struct ImageBarrier {
		GraphicsAPI::Image* image;
		ImageLayout oldLayout;
		ImageLayout newLayout;
		PipelineStage srcStage;
		PipelineStage dstStage;
		AccessFlags srcAccess;
		AccessFlags dstAccess;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 0;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 0;
	};

	/*! CommandBuffers are an object that hold a list of commands to be executed
		by your graphics card. After recording any commands you wish to use for
		a frame, you can send them to a queue to be processed.
	*/
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
		virtual void BeginDebugLabelSection(const char* name, float color[4] = nullptr) = 0;
		virtual void EndDebugLabelSection() = 0;
		virtual void BindGraphicsDescriptorSet(
			const GraphicsPipeline* graphicsPipeline,
			const DescriptorSet* const* descriptorSets,
			uint32_t descriptorSetOffset,
			uint32_t descriptorSetCount
		) = 0;
		virtual void BindComputeDescriptorSet(
			const ComputePipeline* graphicsPipeline,
			const DescriptorSet* const* descriptorSets,
			uint32_t descriptorSetOffset,
			uint32_t descriptorSetCount
		) = 0;
		virtual void BindCommandBuffers(CommandBuffer** commandBuffers, uint32_t commandBuffersCount) = 0;
		virtual void SetViewport(float offsetX, float offsetY, float width, float height, float depthMin = 0.0f, float depthMax = 1.0f) = 0;
		virtual void SetScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) = 0;
		virtual void SetDepthBias(float biasConstantFactor, float biasSlopeFactor) = 0;
		virtual void BindGraphicsPipeline(const GraphicsPipeline* pipeline) = 0;
		virtual void BindComputePipeline(const ComputePipeline* pipeline) = 0;
		virtual void BindVertexArrayObject(const VertexArrayObject* vertexArrayObject) = 0;
		virtual void BindVertexBuffers(const Buffer* const * vb, uint32_t count) = 0;
		virtual void BindIndexBuffer(Buffer* indexBuffer) = 0;
		virtual void DrawVertices(uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) = 0;
		virtual void DrawIndices(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount, int32_t vertexOffset) = 0;
		virtual void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
		virtual void BlitImage(
			Image* src, Image* dst,
			Grindstone::GraphicsAPI::ImageLayout oldLayout,
			Grindstone::GraphicsAPI::ImageLayout newLayout,
			uint32_t width, uint32_t height, uint32_t depth
		) = 0;

		virtual void WaitForComputeMemoryBarrier(Image* renderTarget, bool shouldMakeWritable) = 0;
		virtual void PipelineBarrier(const GraphicsAPI::ImageBarrier* barriers, uint32_t barrierCount) = 0;

		virtual void EndCommandBuffer() = 0;

		struct CommandBufferSecondaryInfo {
			bool isSecondary = false;
			Framebuffer *framebuffer = nullptr;
			RenderPass *renderPass = nullptr;
		};

		struct CreateInfo {
			const char* debugName = nullptr;
			CommandBufferSecondaryInfo secondaryInfo{};
		};
	};
}

inline Grindstone::GraphicsAPI::AccessFlags operator|(Grindstone::GraphicsAPI::AccessFlags a, Grindstone::GraphicsAPI::AccessFlags b) {
	return static_cast<Grindstone::GraphicsAPI::AccessFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline Grindstone::GraphicsAPI::AccessFlags operator&(Grindstone::GraphicsAPI::AccessFlags a, Grindstone::GraphicsAPI::AccessFlags b) {
	return static_cast<Grindstone::GraphicsAPI::AccessFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
