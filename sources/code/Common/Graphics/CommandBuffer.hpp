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

	enum class AccessFlags : uint32_t {
		None = 0,
		IndirectCommandRead = 0x00000001,
		IndexRead = 0x00000002,
		VertexAttributeRead = 0x00000004,
		UNIFORMRead = 0x00000008,
		InputAttachmentRead = 0x00000010,
		ShaderRead = 0x00000020,
		ShaderWrite = 0x00000040,
		ColorAttachmentRead = 0x00000080,
		ColorAttachmentWrite = 0x00000100,
		DepthStencilAttachmentRead = 0x00000200,
		DepthStencilAttachmentWrite = 0x00000400,
		TransferRead = 0x00000800,
		TransferWrite = 0x00001000,
		HostRead = 0x00002000,
		HostWrite = 0x00004000,
		MemoryRead = 0x00008000,
		MemoryWrite = 0x00010000,
		TransformFeedbackWrite = 0x02000000,
		TransformFeedbackCounterRead = 0x04000000,
		TransformFeedbackCounterWrite = 0x08000000,
		ConditionalRenderingRead = 0x00100000,
		ColorAttachmentReadNoncoherent = 0x00080000,
		AccelerationStructureRead = 0x00200000,
		AccelerationStructureWrite = 0x00400000,
		FragmentDensityMapRead = 0x01000000,
		FragmentShadingRateAttachmentRead = 0x00800000,
		CommandPreprocessRead = 0x00020000,
		CommandPreprocessWrite = 0x00040000
	};

	struct ImageBarrier {
		GraphicsAPI::Image* image = nullptr;
		ImageLayout oldLayout;
		ImageLayout newLayout;
		AccessFlags srcAccess;
		AccessFlags dstAccess;
		ImageAspectBits imageAspect;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 0;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 0;
	};

	struct BufferBarrier {
		GraphicsAPI::Buffer* buffer = nullptr;
		AccessFlags srcAccess;
		AccessFlags dstAccess;
		uint32_t offset = 0;
		uint32_t size = 0;
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

		virtual void PipelineBarrier(
			GraphicsAPI::PipelineStageBit srcPipelineStageMask, GraphicsAPI::PipelineStageBit dstPipelineStageMask,
			const GraphicsAPI::BufferBarrier* bufferBarriers, uint32_t bufferBarrierCount,
			const GraphicsAPI::ImageBarrier* imageBarriers, uint32_t imageBarrierCount
		) = 0;

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
