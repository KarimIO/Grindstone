#pragma once

#include <vector>
#include <functional>

#include <Common/HashedString.hpp>
#include "RenderGraphContext.hpp"
#include "RenderGraphPass.hpp"
#include "GpuPassType.hpp"
#include "AttachmentInfo.hpp"
#include "BufferInfo.hpp"

namespace Grindstone::GraphicsAPI {
	class Sampler;
}

namespace Grindstone::Renderer {
	class RenderGraphBuilder;

	class RenderGraphBuilderPass {
	public:
		Grindstone::String name;
		PassId passIndex;
		GpuPassType type;

		Grindstone::Renderer::RenderGraphBuilder* renderGraphBuilder = nullptr;

		std::vector<GraphicsAPI::Sampler*> samplers;
		std::vector<PassBufferDesc> bufferRefs;
		std::vector<PassImageDesc> imageRefs;

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const = 0;
	};

	class PipelineRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		void ReadExternalSampler(Grindstone::GraphicsAPI::Sampler* sampler);
		void ReadSampledImage(RenderGraphBuilderResourceRef inputHandle);
		void ReadBuffer(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef ReadWriteBuffer(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteBuffer(BufferDescription resource);
	};

	class GraphicsRenderGraphBuilderPassBase : public PipelineRenderGraphBuilderPass {
	public:
		RenderGraphBuilderResourceRef ReadWriteColorAttachment(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteColorAttachment(ImageDescription resource, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearColor clearValue);
		RenderGraphBuilderResourceRef WriteColorAttachment(RenderGraphBuilderResourceRef ref, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearColor clearValue);

		void ReadDepthAttachment(RenderGraphBuilderResourceRef inputHandle);
		void ReadDepthAttachmentSampled(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef ReadWriteDepthStencilAttachment(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteDepthStencilAttachment(ImageDescription resource, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);
		Grindstone::Renderer::RenderGraphBuilderResourceRef WriteDepthStencilAttachment(RenderGraphBuilderResourceRef resource, Grindstone::GraphicsAPI::LoadOp loadOp, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

	protected:

		MetaRect renderingArea;

	};

	template<typename ReturnType>
	class GraphicsRenderGraphBuilderPass : public GraphicsRenderGraphBuilderPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Math::IntRect2D, Grindstone::Renderer::RenderGraphContext&, const Grindstone::Renderer::RenderGraphFrameResources&, ReturnType&)>;

		void SetRenderingArea(
			MetaRect renderingArea
		) {
			this->renderingArea = renderingArea;
		}

		void SetExecutionCallback(ExecutionCallbackFn callback) {
			executionCallback = callback;
		}

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const override {
			auto pass = Grindstone::Memory::AllocatorCore::AllocateUnique<GraphicsRenderGraphPass<ReturnType>>();
			pass->name = name;
			pass->type = type;
			pass->executionCallback = executionCallback;
			pass->metaRenderingArea = renderingArea;
			pass->samplers = samplers;
			pass->returnData = returnData;
			pass->imageDescs = imageRefs;
			pass->bufferDescs = bufferRefs;
			return pass;
		}

		ReturnType returnData;
		ExecutionCallbackFn executionCallback;

	};

	class ComputeRenderGraphBuilderPassBase : public PipelineRenderGraphBuilderPass {
	public:

		void ReadStorageImage(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef ReadWriteStorageImage(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteStorageImage(ImageDescription resource);

	};

	template<typename ReturnType>
	class ComputeRenderGraphBuilderPass : public ComputeRenderGraphBuilderPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, const Grindstone::Renderer::RenderGraphFrameResources& frameResources, ReturnType&)>;

		void SetExecutionCallback(ExecutionCallbackFn callback) {
			executionCallback = callback;
		}

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const override {
			auto pass = Grindstone::Memory::AllocatorCore::AllocateUnique<ComputeRenderGraphPass<ReturnType>>();
			pass->name = name;
			pass->type = type;
			pass->executionCallback = executionCallback;
			pass->samplers = samplers;
			pass->returnData = returnData;
			pass->imageDescs = imageRefs;
			pass->bufferDescs = bufferRefs;
			return pass;
		}

		ReturnType returnData;
		ExecutionCallbackFn executionCallback;

	};

	struct BuilderImageTransfer {
		RenderGraphBuilderResourceRef dstImage;
		RenderGraphBuilderResourceRef srcImage;
		GraphicsAPI::TextureFilter filter;
		Grindstone::Math::IntBox3D srcRegion;
		Grindstone::Math::IntBox3D dstRegion;
	};

	struct BuilderBufferTransfer {
		RenderGraphBuilderResourceRef dstBuffer;
		RenderGraphBuilderResourceRef srcBuffer;
		uint64_t dstOffset;
		uint64_t srcOffset;
		uint64_t size;
	};

	class TransferRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		virtual void AddImageTransfer(BuilderImageTransfer transfer);
		virtual void AddBufferTransfer(BuilderBufferTransfer transfer);

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const override {
			auto pass = Grindstone::Memory::AllocatorCore::AllocateUnique<TransferRenderGraphPass>();
			pass->name = name;
			pass->type = type;
			return pass;
		}

		std::vector<BuilderImageTransfer> imageTransfers;
		std::vector<BuilderBufferTransfer> bufferTransfers;
	};

	class PresentRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		virtual void SetPresentationImage(RenderGraphBuilderResourceRef targetImage);

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const override {
			auto pass = Grindstone::Memory::AllocatorCore::AllocateUnique<PresentRenderGraphPass>();
			pass->name = name;
			pass->type = type;
			return pass;
		}

	protected:
		RenderGraphBuilderResourceRef targetImage;

	};

}
