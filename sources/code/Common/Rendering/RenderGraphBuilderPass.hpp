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
	using PassId = uint16_t;
	using ResourceId = uint16_t;
	const PassId invalidPassId = std::numeric_limits<PassId>().max();
	const ResourceId invalidResourceId = std::numeric_limits<ResourceId>().max();

	class RenderGraphBuilder;

	struct RenderGraphBuilderResourceRef {
		ResourceId isBuffer : 1;
		ResourceId resourceIndex : 15;
		PassId passIndex;

		static_assert(sizeof(ResourceId) == 2);

		static RenderGraphBuilderResourceRef Invalid() {
			return {
				.isBuffer = 0,
				.resourceIndex = static_cast<uint16_t>(0xffff),
				.passIndex = 0xffff
			};
		}

		static RenderGraphBuilderResourceRef Buffer(ResourceId resourceIndex, PassId passIndex) {
			return {
				.isBuffer = 1,
				.resourceIndex = resourceIndex,
				.passIndex = passIndex
			};
		}

		static RenderGraphBuilderResourceRef Image(ResourceId resourceIndex, PassId passIndex) {
			return {
				.isBuffer = 0,
				.resourceIndex = resourceIndex,
				.passIndex = passIndex
			};
		}

		bool IsBuffer() const {
			return isBuffer == 1;
		}

		bool IsImage() const {
			return isBuffer == 0;
		}

		ResourceId GetResourceIndex() const {
			return resourceIndex;
		}

		PassId GetPassIndex() const {
			return passIndex;
		}

		RenderGraphBuilderResourceRef FromPass(PassId newPassIndex) const {
			return {
				.isBuffer = isBuffer,
				.resourceIndex = resourceIndex,
				.passIndex = newPassIndex
			};
		}

		bool IsSameResource(const RenderGraphBuilderResourceRef& other) const {
			return
				isBuffer == other.isBuffer &&
				resourceIndex == other.resourceIndex;
		}

		bool operator==(const RenderGraphBuilderResourceRef& o) const {
			return
				(isBuffer == o.isBuffer) &&
				(resourceIndex == o.resourceIndex) &&
				(passIndex == o.passIndex);
		}
	};

	enum class AccessType {
		Read,
		Write,
		ReadWrite
	};

	struct PassBufferDesc {
		RenderGraphBuilderResourceRef ref;
		AccessType accessType;
	};

	struct PassImageDesc {
		RenderGraphBuilderResourceRef ref;
		AccessType accessType;
	};

	class RenderGraphBuilderPass {
	public:
		Grindstone::String name;
		PassId passIndex;
		GpuPassType type;

		Grindstone::Renderer::RenderGraphBuilder* renderGraphBuilder = nullptr;

		std::vector<PassBufferDesc> bufferRefs;
		std::vector<PassImageDesc> imageRefs;

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const = 0;
	};

	class PipelineRenderGraphBuilderPass : public RenderGraphBuilderPass {
	public:
		void ReadImage(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef ReadWriteImage(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteImage(ImageDescription resource);

		void ReadBuffer(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef ReadWriteBuffer(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteBuffer(BufferDescription resource);
	};

	class GraphicsRenderGraphBuilderPassBase : public PipelineRenderGraphBuilderPass {
	public:
		RenderGraphBuilderResourceRef ReadWriteColorAttachment(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteColorAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue);

		RenderGraphBuilderResourceRef ReadWriteDepthStencilAttachment(RenderGraphBuilderResourceRef inputHandle);
		RenderGraphBuilderResourceRef WriteDepthStencilAttachment(ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue);

	protected:
		std::vector<AttachmentInfo> attachmentInfo;
		MetaSize xOffset;
		MetaSize yOffset;
		MetaSize width;
		MetaSize height;

	};

	template<typename ReturnType>
	class GraphicsRenderGraphBuilderPass : public GraphicsRenderGraphBuilderPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Math::IntRect2D, Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::GraphicsRenderGraphPass<ReturnType>&, ReturnType&)>;

		void SetRenderingArea(
			MetaSize xOffset,
			MetaSize yOffset,
			MetaSize width,
			MetaSize height
		) {
			this.xOffset = xOffset;
			this.yOffset = yOffset;
			this.width = width;
			this.height = height;
		}

		void SetExecutionCallback(ExecutionCallbackFn callback) {
			executionCallback = callback;
		}

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const override {
			auto pass = Grindstone::Memory::AllocatorCore::AllocateUnique<GraphicsRenderGraphPass<ReturnType>>();
			pass->name = name;
			pass->type = type;
			pass->executionCallback = executionCallback;
			return pass;
		}

	protected:
		ExecutionCallbackFn executionCallback;

	};

	class ComputeRenderGraphBuilderPassBase : public PipelineRenderGraphBuilderPass {
	public:


	};

	template<typename ReturnType>
	class ComputeRenderGraphBuilderPass : public ComputeRenderGraphBuilderPassBase {
	public:
		using ExecutionCallbackFn = std::function<void(Grindstone::Renderer::RenderGraphContext&, Grindstone::Renderer::ComputeRenderGraphPass<ReturnType>&, ReturnType&)>;

		void SetExecutionCallback(ExecutionCallbackFn callback) {
			executionCallback = callback;
		}

		virtual Grindstone::UniquePtr<RenderGraphPass> ConstructExecutionPass() const override {
			auto pass = Grindstone::Memory::AllocatorCore::AllocateUnique<ComputeRenderGraphPass<ReturnType>>();
			pass->name = name;
			pass->type = type;
			pass->executionCallback = executionCallback;
			return pass;
		}

	protected:
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

template<>
struct std::hash<Grindstone::Renderer::RenderGraphBuilderResourceRef> {
	size_t operator()(const Grindstone::Renderer::RenderGraphBuilderResourceRef& h) const {
		// Pack the bitfields + passIndex into a single integer to hash
		uint32_t packed = (h.isBuffer << 15) | h.resourceIndex;

		return std::hash<uint64_t>{}(
			(static_cast<uint64_t>(packed) << 16) | h.passIndex
		);
	}
};
