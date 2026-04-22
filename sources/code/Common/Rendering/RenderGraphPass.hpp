#pragma once

#include <vector>
#include <functional>

#include <Common/HashedString.hpp>
#include <Common/Graphics/CommandBuffer.hpp>
#include "GpuPassType.hpp"
#include "RenderGraphContext.hpp"
#include "AttachmentInfo.hpp"
#include "BufferInfo.hpp"

namespace Grindstone::Renderer {

	struct RenderGraphResourceRef {
		uint16_t isBuffer : 1;
		uint16_t resourceIndex : 15;

		static_assert(sizeof(uint16_t) == 2);

		static RenderGraphResourceRef Buffer(uint16_t resourceIndex) {
			return {
				.isBuffer = 1,
				.resourceIndex = resourceIndex
			};
		}

		static RenderGraphResourceRef Image(uint16_t resourceIndex) {
			return {
				.isBuffer = 0,
				.resourceIndex = resourceIndex
			};
		}

		bool IsBuffer() const {
			return isBuffer == 1;
		}

		bool IsImage() const {
			return isBuffer == 0;
		}

		uint16_t GetResourceIndex() const {
			return resourceIndex;
		}

		bool operator==(const RenderGraphResourceRef& o) const {
			return
				(isBuffer == o.isBuffer) &&
				(resourceIndex == o.resourceIndex);
		}
	};

	class RenderGraphPass {
	public:
		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) = 0;

		Grindstone::String name;
		GpuPassType type;

		std::vector<Grindstone::GraphicsAPI::ImageBarrier> imageBarriers;
		std::vector<Grindstone::GraphicsAPI::BufferBarrier> bufferBarriers;

	protected:

		void SubmitBarriers(Grindstone::Renderer::RenderGraphContext& context);

	};

	class PipelineRenderGraphPass : public RenderGraphPass {
	public:

		Grindstone::GraphicsAPI::DescriptorSet* descriptorSet = nullptr;

	};

	class GraphicsRenderGraphPassBase : public PipelineRenderGraphPass {
	public:

		Grindstone::Math::IntRect2D renderingArea;
		std::vector<Grindstone::GraphicsAPI::RenderAttachment> colorAttachments;
		Grindstone::GraphicsAPI::RenderAttachment depthAttachment;
		bool hasDepthAttachment = false;

	protected:

		void PrepareGraphicsPass(Grindstone::Renderer::RenderGraphContext& context);
		void EndGraphicsPass(Grindstone::Renderer::RenderGraphContext& context);

	};

	template<typename ReturnType>
	class GraphicsRenderGraphPass : public GraphicsRenderGraphPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Math::IntRect2D, Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::GraphicsRenderGraphPass<ReturnType>&, ReturnType&)>;
		ExecutionCallbackFn executionCallback;
		ReturnType returnType;

		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) override {
			GS_ASSERT_ENGINE_WITH_MESSAGE(executionCallback != nullptr, "Execution callback for rendergraph pass %s is not set.", name.c_str());

			PrepareGraphicsPass(context);
			executionCallback(renderingArea, context, *this, returnType);
			EndGraphicsPass(context);
		}

	};

	class ComputeRenderGraphPassBase : public PipelineRenderGraphPass {
	public:

	protected:

		void PrepareComputePass(Grindstone::Renderer::RenderGraphContext& context);

	};

	template<typename ReturnType>
	class ComputeRenderGraphPass : public ComputeRenderGraphPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::ComputeRenderGraphPass<ReturnType>&, ReturnType&)>;
		ExecutionCallbackFn executionCallback;
		ReturnType returnType;

		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) override {
			GS_ASSERT_ENGINE_WITH_MESSAGE(executionCallback != nullptr, "Execution callback for rendergraph pass %s is not set.", name.c_str());

			PrepareComputePass(context);
			executionCallback(context, *this, returnType);
		}

	};

	struct ImageTransfer {
		Grindstone::GraphicsAPI::Image* dst;
		Grindstone::GraphicsAPI::Image* src;
		GraphicsAPI::TextureFilter filter;
		Grindstone::Math::IntBox3D srcRegion;
		Grindstone::Math::IntBox3D dstRegion;
	};

	struct BufferTransfer {
		Grindstone::GraphicsAPI::Buffer* dstBuff;
		Grindstone::GraphicsAPI::Buffer* srcBuff;
		uint64_t dstOffset;
		uint64_t srcOffset;
		uint64_t size;
	};

	class TransferRenderGraphPass : public RenderGraphPass {
	public:
		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) override;

	protected:
		std::vector<ImageTransfer> imageTransfers;
		std::vector<BufferTransfer> bufferTransfers;
	};

	class PresentRenderGraphPass : public RenderGraphPass {
	public:
		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) override;

	};
}
