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
	using ResourceWrite = size_t;
	using ResourceRead = size_t;
	using ResourceReadWrite = size_t;

	class RenderGraphPass {
	public:
		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) = 0;

		Grindstone::HashedString name;

		GpuPassType type;

		std::vector<ResourceWrite> writes;
		std::vector<ResourceRead> reads;
		std::vector<ResourceReadWrite> readWrites;

		std::vector<Grindstone::GraphicsAPI::ImageBarrier> imageBarriers;
		std::vector<Grindstone::GraphicsAPI::BufferBarrier> bufferBarriers;

	protected:

		void SubmitBarriers(Grindstone::Renderer::RenderGraphContext& context);

	};

	class PipelineRenderGraphPass : public RenderGraphPass {
	public:
		void ReadStorageImage(Grindstone::HashedString inputName, ImageDescription resource);
		void ReadWriteStorageImage(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
		void WriteStorageImage(Grindstone::HashedString outputName, ImageDescription resource);

		void ReadBuffer(Grindstone::HashedString inputname, BufferDescription resource);
		void WriteBuffer(Grindstone::HashedString outputName, BufferDescription resource);
		void ReadWriteBuffer(Grindstone::HashedString inputName, Grindstone::HashedString outputName, BufferDescription resource);

	};

	class GraphicsRenderGraphPassBase : public PipelineRenderGraphPass {
	public:
		void ReadColorAttachment(Grindstone::HashedString inputName, ImageDescription resource);
		void ReadWriteColorAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
		void WriteColorAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue);

		void ReadDepthStencilAttachment(Grindstone::HashedString inputName, ImageDescription resource);
		void ReadWriteDepthStencilAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
		void WriteDepthStencilAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

	protected:

		void PrepareGraphicsPass(Grindstone::Renderer::RenderGraphContext& context);
		void EndGraphicsPass(Grindstone::Renderer::RenderGraphContext& context);

	};

	template<typename ReturnType>
	class GraphicsRenderGraphPass : public GraphicsRenderGraphPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::GraphicsRenderGraphPass<ReturnType>&, ReturnType&)>;

		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) override {
			PrepareGraphicsPass(context);
			executionCallback(context, this, returnType);
			EndGraphicsPass(context);
		}

	protected:
		ExecutionCallbackFn executionCallback;
		ReturnType returnType;

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

		virtual void Execute(Grindstone::Renderer::RenderGraphContext& context) override {
			PrepareComputePass(context);

			executionCallback(context, this, returnType);
		}

	protected:
		ExecutionCallbackFn executionCallback;
		ReturnType returnType;

	};

	using TGResourceHandle = uint32_t;

	struct ImageTransfer {
		TGResourceHandle dst;
		TGResourceHandle src;
		GraphicsAPI::TextureFilter filter;
	};

	struct BufferTransfer {
		TGResourceHandle dstBuff;
		TGResourceHandle srcBuff;
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
