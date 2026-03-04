#pragma once

#include <vector>
#include <functional>

#include <Common/HashedString.hpp>
#include "RenderGraphContext.hpp"
#include "RenderGraphPass.hpp"
#include "GpuPassType.hpp"
#include "AttachmentInfo.hpp"
#include "BufferInfo.hpp"

namespace Grindstone::Renderer {
	struct TGBImageRef {
		TGResourceHandle index;

		bool operator==(const TGBImageRef& o) const { return index == o.index; }
	};

	struct TGBBufferRef {
		TGResourceHandle index;

		bool operator==(const TGBBufferRef& o) const { return index == o.index; }
	};

	class RenderGraphBuilderPass {
	public:
		Grindstone::String name;

		GpuPassType type;

		std::vector<ResourceWrite> writes;
		std::vector<ResourceRead> reads;
		std::vector<ResourceReadWrite> readWrites;
	};

	class PipelineRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		void ReadImage(TGBImageRef inputHandle);
		TGBImageRef ReadWriteImage(TGBImageRef inputHandle);
		TGBImageRef WriteImage(ImageDescription resource);

		void ReadBuffer(TGBBufferRef inputHandle);
		TGBBufferRef ReadWriteBuffer(TGBBufferRef inputHandle);
		TGBBufferRef WriteBuffer(BufferDescription resource);
	};

	class GraphicsRenderGraphBuilderPassBase : public PipelineRenderGraphBuilderPass {
	public:
		void ReadColorAttachment(TGBImageRef inputHandle);
		TGBImageRef ReadWriteColorAttachment(TGBImageRef inputHandle);
		TGBImageRef WriteColorAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue);

		void ReadDepthStencilAttachment(TGBImageRef inputHandle);
		TGBImageRef ReadWriteDepthStencilAttachment(TGBImageRef inputHandle);
		TGBImageRef WriteDepthStencilAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

	protected:
		std::vector<AttachmentInfo> attachmentInfo;

	};

	template<typename ReturnType>
	class GraphicsRenderGraphBuilderPass : public GraphicsRenderGraphBuilderPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::GraphicsRenderGraphPass<ReturnType>&, ReturnType&)>;

		void SetExecutionCallback(ExecutionCallbackFn callback) {
			execution = callback;
		}

	protected:
		ExecutionCallbackFn execution;

	};

	class ComputeRenderGraphBuilderPassBase : public PipelineRenderGraphBuilderPass {
	public:


	};

	template<typename ReturnType>
	class ComputeRenderGraphBuilderPass : public ComputeRenderGraphBuilderPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::ComputeRenderGraphPass<ReturnType>&, ReturnType&)>;

		void SetExecutionCallback(ExecutionCallbackFn callback) {
			execution = callback;
		}

	protected:
		ExecutionCallbackFn execution;

	};

	struct ImageTransfer {
		TGResourceHandle dstImage;
		TGResourceHandle srcImage;
		GraphicsAPI::TextureFilter filter;
		Grindstone::Math::IntRect2D srcOffset;
		Grindstone::Math::IntRect2D dstOffset;
	};

	struct BufferTransfer {
		TGResourceHandle dstBuffer;
		TGResourceHandle srcBuffer;
		uint64_t dstOffset;
		uint64_t srcOffset;
		uint64_t size;
	};

	class TransferRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		virtual void AddImageTransfer(ImageTransfer transfer);
		virtual void AddBufferTransfer(BufferTransfer transfer);

		std::vector<ImageTransfer> imageTransfers;
		std::vector<BufferTransfer> bufferTransfers;
	};

	class PresentRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		virtual void SetPresentationImage(TGBImageRef targetImage);

	protected:
		TGBImageRef targetImage;

	};

}
