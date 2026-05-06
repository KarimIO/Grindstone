#pragma once

#include <vector>
#include <variant>
#include <functional>

#include <Common/HashedString.hpp>
#include <Common/Graphics/CommandBuffer.hpp>

#include "GpuPassType.hpp"
#include "RenderGraphContext.hpp"
#include "AttachmentInfo.hpp"
#include "BufferInfo.hpp"
#include "RenderGraphFrameResources.hpp"

namespace Grindstone::Renderer {
	using UnionResourceDescription = std::variant<Grindstone::Renderer::ImageDescription, Grindstone::Renderer::BufferDescription>;

	class RenderGraphPass {
	public:
		virtual void RealizeResources(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) = 0;
		virtual void Execute(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) = 0;

		Grindstone::String name;
		GpuPassType type;

		std::vector<Grindstone::GraphicsAPI::ImageBarrier> imageBarriers;
		std::vector<Grindstone::GraphicsAPI::BufferBarrier> bufferBarriers;

	protected:

		void SubmitBarriers(Grindstone::Renderer::RenderGraphContext& context);

	};

	class PipelineRenderGraphPass : public RenderGraphPass {
	public:

		std::vector<PassImageDesc> imageDescs;
		std::vector<PassBufferDesc> bufferDescs;
		Grindstone::GraphicsAPI::DescriptorSet* passDescriptorSet = nullptr;
		Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = nullptr;

		virtual void RealizeResources(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override;

	};

	class GraphicsRenderGraphPassBase : public PipelineRenderGraphPass {
	public:

		Grindstone::Renderer::MetaRect metaRenderingArea;

		virtual Grindstone::Math::IntRect2D PrepareGraphicsPass(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		);
		virtual void EndGraphicsPass(Grindstone::Renderer::RenderGraphContext& context);

	};

	template<typename ReturnType>
	class GraphicsRenderGraphPass : public GraphicsRenderGraphPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Math::IntRect2D, Grindstone::Renderer::RenderGraphContext&, const Grindstone::Renderer::RenderGraphFrameResources&, ReturnType&)>;
		ExecutionCallbackFn executionCallback;
		ReturnType returnData;

		virtual void Execute(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override {
			GS_ASSERT_ENGINE_WITH_MESSAGE(executionCallback != nullptr, "Execution callback for rendergraph pass %s is not set.", name.c_str());

			Grindstone::Math::IntRect2D renderingArea = PrepareGraphicsPass(context, frameResources);
			executionCallback(renderingArea, context, frameResources, returnData);
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
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, const Grindstone::Renderer::RenderGraphFrameResources&, ReturnType&)>;
		ExecutionCallbackFn executionCallback;
		ReturnType returnData;

		virtual void Execute(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override {
			GS_ASSERT_ENGINE_WITH_MESSAGE(executionCallback != nullptr, "Execution callback for rendergraph pass %s is not set.", name.c_str());

			PrepareComputePass(context);
			executionCallback(context, frameResources, returnData);
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

		virtual void RealizeResources(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override;
		virtual void Execute(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override;

	protected:
		std::vector<ImageTransfer> imageTransfers;
		std::vector<BufferTransfer> bufferTransfers;
	};

	class PresentRenderGraphPass : public RenderGraphPass {
	public:
		virtual void RealizeResources(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override;
		virtual void Execute(
			Grindstone::Renderer::RenderGraphContext& context,
			Grindstone::Renderer::RenderGraphFrameResources& frameResources
		) override;

	};
}
