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
	class RenderGraphBuilderPass {
	public:
		Grindstone::HashedString name;

		GpuPassType type;

		std::vector<ResourceWrite> writes;
		std::vector<ResourceRead> reads;
		std::vector<ResourceReadWrite> readWrites;

		std::function<void(RenderGraphContext&, Grindstone::Renderer::RenderGraph::RenderPassExecution&)> execution;
	};

	class PipelineRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		void ReadImage(Grindstone::HashedString inputName, ImageDescription resource);
		void ReadWriteStorageImage(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
		void WriteImage(Grindstone::HashedString outputName, ImageDescription resource);

		void ReadBuffer(Grindstone::HashedString inputname, BufferDescription resource);
		void WriteBuffer(Grindstone::HashedString outputName, BufferDescription resource);
		void ReadWriteBuffer(Grindstone::HashedString inputName, Grindstone::HashedString outputName, BufferDescription resource);

		std::vector<ResourceWrite> writes;
		std::vector<ResourceRead> reads;
		std::vector<ResourceReadWrite> readWrites;
	};

	class GraphicsRenderGraphBuilderPass : public PipelineRenderGraphBuilderPass {
	public:
		void ReadColorAttachment(Grindstone::HashedString inputName, ImageDescription resource);
		void ReadWriteColorAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
		void WriteColorAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue);

		void ReadDepthStencilAttachment(Grindstone::HashedString inputName, ImageDescription resource);
		void ReadWriteDepthStencilAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource);
		void WriteDepthStencilAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

		void SetExecutionCallback(GraphicsExecutionCallback callback);

	protected:
		GraphicsExecutionCallback execution;

	};

	class ComputeRenderGraphBuilderPass : public PipelineRenderGraphBuilderPass {
	public:

		void SetExecutionCallback(ComputeExecutionCallback callback);

	protected:
		ComputeExecutionCallback execution;

	};

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

	class TransferRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:


	};

	class PresentRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:


	protected:
		std::function<void(RenderGraphContext&, Grindstone::Renderer::RenderGraph::RenderPassExecution&)> execution;

	};

}
