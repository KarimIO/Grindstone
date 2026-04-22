#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Graphics/Core.hpp>

#include "TransientResourceManager.hpp"
#include "RenderGraphBuilder.hpp"

using namespace Grindstone::Renderer;

// ===============================================================
// - Setup Phase
// ===============================================================

// TODO: These should be allocated on a linear allocator.

TransferRenderGraphBuilderPass* RenderGraphBuilder::CreateTransferPass(
	Grindstone::StringRef name,
	std::function<void(TransferRenderGraphBuilderPass&)> setupImmediateCallback
) {
	auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<TransferRenderGraphBuilderPass>());
	auto pass = static_cast<TransferRenderGraphBuilderPass*>(uniquePtr.Get());
	pass->name = name;
	pass->renderGraphBuilder = this;
	setupImmediateCallback(*pass);
	return pass;
}

const char* presentPassName = "Present to Screen";
PresentRenderGraphBuilderPass* RenderGraphBuilder::CreatePresentPass(RenderGraphBuilderResourceRef imageRef) {
	auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<PresentRenderGraphBuilderPass>());
	auto pass = static_cast<PresentRenderGraphBuilderPass*>(uniquePtr.Get());
	pass->name = presentPassName;
	pass->renderGraphBuilder = this;
	pass->SetPresentationImage(imageRef);
	return pass;
}

void RenderGraphBuilder::CreatePresentPass(
	std::function<void(PresentRenderGraphBuilderPass&)> setupImmediateCallback
) {
	auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<PresentRenderGraphBuilderPass>());
	auto pass = static_cast<PresentRenderGraphBuilderPass*>(uniquePtr.Get());
	pass->name = presentPassName;
	pass->renderGraphBuilder = this;
	setupImmediateCallback(*pass);
}

// ===============================================================
// - Compile Phase
// ===============================================================

using RenderGraphPassList = std::vector<Grindstone::UniquePtr<RenderGraphBuilderPass>>;

static void SetupBarriers();
static bool OptimizeRenderGraph(
	const RenderGraphPassList& passes,
	std::vector<PassId>& sortedAndCulledPasses
);
static void RealizeResources();
static void SetupAttachments();
static void CreatePasses(
	const RenderGraphPassList& passes,
	const std::vector<PassId>& sortedPassesIndices,
	std::vector<Grindstone::UniquePtr<RenderGraphPass>>& compiledPasses
);

RenderGraphBuilderResourceRef RenderGraphBuilder::AddImage(ImageDescription imageDesc, Renderer::PassId passId) {
	size_t imageIndex = images.size();
	images.emplace_back(imageDesc);

	return {
		.resourceIndex = static_cast<Renderer::ResourceId>(imageIndex),
		.passIndex = passId
	};
}

RenderGraphBuilderResourceRef RenderGraphBuilder::AddBuffer(BufferDescription bufferDesc, Renderer::PassId passId) {
	size_t bufferIndex = buffers.size();
	buffers.emplace_back(bufferDesc);

	return {
		.resourceIndex = static_cast<Renderer::ResourceId>(bufferIndex),
		.passIndex = passId
	};
}

RenderGraph RenderGraphBuilder::Compile() const {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	std::vector<PassId> sortedPassesIndices;

	for (PassId i = 0; i < static_cast<PassId>(passes.size()); ++i) {
		sortedPassesIndices.emplace_back(i);
	}

	bool isGraphValid = true; // OptimizeRenderGraph(passes, sortedPassesIndices);
	if (!isGraphValid) {
		GPRINT_ERROR(LogSource::RenderingBackend, "Render graph has a cycle - we cannot compile it. Ensure passes do not depend on passes that depend on themselves");
		throw "";
		// return RenderGraph();
	}

	std::vector<Grindstone::UniquePtr<RenderGraphPass>> compiledPasses;
	CreatePasses(passes, sortedPassesIndices, compiledPasses);
	SetupBarriers();
	RealizeResources();
	SetupAttachments();

	return RenderGraph(std::move(compiledPasses));
}

void RenderGraphBuilder::Clear() {
	passes.clear();
	images.clear();
	buffers.clear();
	presentationResourceId = invalidResourceId;
}

/*!
	\brief Sort and cull the Render Graph so that we can execute them in order based off of the resources of each pass.
	\param passes The list of all render graph builder passes.
	\param sortedAndCulledPasses The output indices of each pass, culled and sorted.
	\return Whether the render graph is valid (ie has no cyclic dependencies).
*/
/*
static bool OptimizeRenderGraph(
	const RenderGraphPassList& passes,
	std::vector<PassId>& sortedAndCulledPasses
) {
	using namespace Grindstone::Renderer;

	PassId totalPassCount = static_cast<PassId>(passes.size());
	std::vector<PassId> indegree(totalPassCount, 0);

	size_t n = passes.size();

	passDependencies.resize(n);
	for (size_t index = 0; index < n; ++index) {
		const Grindstone::UniquePtr<RenderGraphBuilderPass>& pass = passes[index];

		for (const PassBufferDesc& buffer : pass->bufferRefs) {
			passDependencies[index].insert(buffer.ref.passIndex);
		}

		for (const PassImageDesc& image : pass->imageRefs) {
			passDependencies[index].insert(image.ref.passIndex);
		}

		if (pass->type == )
	}

	std::set<PassId> usedPasses;
	std::queue<PassId> passesToProcess;

	for (PassId passId : externalOutputs) {
		passesToProcess.push(passId);
	}

	// For each resource in resources queue:
	while (!passesToProcess.empty()) {
		PassId passId = passesToProcess.front();
		passesToProcess.pop();

		// Don't iterate if we already used this pass.
		if (usedPasses.contains(passId)) {
			continue;
		}

		// Mark this resource's source pass as used.
		usedPasses.insert(passId);

		const std::set<PassId>& passInputs = passDependencies[passId];
		for (PassId passInput : passInputs) {
			passesToProcess.push(passInput);
		}
	}

	// Compute indegrees (number of dependencies) for each pass.
	for (PassId i = 0; i < totalPassCount; i++) {
		for (PassId next : passes[i].tex) {
			indegree[next]++;
		}
	}

	std::queue<PassId> queue;

	// Add all nodes with indegree 0 into the front of the queue
	// because they should be as early as possible.
	for (PassId i = 0; i < totalPassCount; i++) {
		if (indegree[i] == 0) {
			queue.push(i);
		}
	}

	// Kahn's Algorithm (BFS)
	while (!queue.empty()) {
		PassId top = queue.front();
		queue.pop();
		sortedOutput.push_back(top);
		for (PassId next : graph[top]) {
			indegree[next]--;
			if (indegree[next] == 0) {
				queue.push(next);
			}
		}
	}

	// A cycle is contained if the generated output does not have the same number of nodes
	// as the input graph.
	return (sortedOutput.size() == totalPassCount);
}
*/

static void CreatePasses(
	const RenderGraphPassList& builderPasses,
	const std::vector<PassId>& sortedPassesIndices,
	std::vector<Grindstone::UniquePtr<RenderGraphPass>>& compiledPasses
) {
	compiledPasses.reserve(sortedPassesIndices.size());

	for (PassId passId : sortedPassesIndices) {
		const Grindstone::UniquePtr<RenderGraphBuilderPass>& builderPass = builderPasses[passId];
		compiledPasses.emplace_back(std::move(builderPass->ConstructExecutionPass()));
	}
}

void SetupBarriers() {}
void SetupAttachments() {}

/*
static std::tuple<
	Grindstone::GraphicsAPI::ImageLayout,
	Grindstone::GraphicsAPI::AccessFlags,
	Grindstone::GraphicsAPI::PipelineStageBit
> InferBarrierData(
	GpuPassType gpuQueue,
	RenderGraphBuilder::ResourceType resourceType,
	AccessType accessType
	// Grindstone::GraphicsAPI::ShaderStageBit stages
) {
	using namespace Grindstone::GraphicsAPI;
	using namespace Grindstone::Renderer;

	if (accessType == AccessType::Read) {
		return {
			ImageLayout::ShaderRead,
			AccessFlags::ShaderRead,
			gpuQueue == GpuPassType::Compute
				? PipelineStageBit::ComputeShader
				: PipelineStageBit::FragmentShader // ShaderStagesToPipelineStages(stages)
		};
	}

	switch (resourceType) {
	case RenderGraph::ResourceType::ColorAttachment:
		return {
			ImageLayout::ColorAttachment,
			AccessFlags::ColorAttachmentWrite,
			PipelineStageBit::ColorAttachmentOutput
		};

	case RenderGraph::ResourceType::DepthAttachment:
		return {
			ImageLayout::DepthWrite,
			AccessFlags::DepthStencilAttachmentWrite,
			PipelineStageBit::EarlyFragmentTests |
			PipelineStageBit::LateFragmentTests
		};
	case RenderGraph::ResourceType::StorageImage:
		return {
			ImageLayout::General,
			AccessFlags::ShaderWrite,
			PipelineStageBit::ComputeShader
		};
	}

	GS_ASSERT_LOG("Invalid resource!");

	return { ImageLayout::Undefined, AccessFlags::None, PipelineStageBit::None };
}

static void SetupBarriers() {
	for (RenderGraphPass& pass : passes) {
		Grindstone::GraphicsAPI::PipelineStageBit srcPipelineStageMask = Grindstone::GraphicsAPI::PipelineStageBit::AllGraphics;
		Grindstone::GraphicsAPI::PipelineStageBit dstPipelineStageMask = Grindstone::GraphicsAPI::PipelineStageBit::None;
		std::vector<Grindstone::GraphicsAPI::ImageBarrier> imageBarriers;
		std::vector<Grindstone::GraphicsAPI::BufferBarrier> bufferBarriers;

		for (ResourceRead& read : pass.reads) {
			switch (read.resource.resourceType) {
			case ResourceType::ColorAttachment:
			case ResourceType::DepthAttachment:
			case ResourceType::StorageImage:
			{
				ImageDescription& imageDesc = read.resource.image;
				ResourceId resourceId = namesToResourceIds[read.name];
				TransientImageData& imageData = transientResourceManager.GetTrackedImage(context.swapchainSize.x, context.swapchainSize.y, imageDesc, trackedResources[resourceId].second);

				GraphicsAPI::ImageLayout srcLayout = imageData.currentLayout;
				GraphicsAPI::AccessFlags srcAccess = imageData.currentAccessFlags;
				GraphicsAPI::ImageLayout dstLayout = GraphicsAPI::ImageLayout::Undefined;
				GraphicsAPI::AccessFlags dstAccess = GraphicsAPI::AccessFlags::None;
				GraphicsAPI::PipelineStageBit dstPipelineStageBit = GraphicsAPI::PipelineStageBit::None;

				std::tie(dstLayout, dstAccess, dstPipelineStageBit) = InferBarrierData(pass.type, read.resource.resourceType, AccessType::Read);

				if (srcLayout != dstLayout && srcAccess != dstAccess) {
					GraphicsAPI::ImageAspectBits aspect = GraphicsAPI::GetFormatAspect(imageDesc.format);
					imageBarriers.emplace_back(
						GraphicsAPI::ImageBarrier{
							.image = imageData.image,
							.oldLayout = srcLayout,
							.newLayout = dstLayout,
							.srcAccess = srcAccess,
							.dstAccess = dstAccess,
							.imageAspect = aspect,
							.baseMipLevel = 0,
							.levelCount = imageDesc.mipLevels,
							.baseArrayLayer = 0,
							.layerCount = imageDesc.arrayLayers
						}
					);

					imageData.currentLayout = dstLayout;
					imageData.currentAccessFlags = dstAccess;
					dstPipelineStageMask |= dstPipelineStageBit;
				}
				break;
			}
			case ResourceType::StorageBuffer:
			case ResourceType::UniformBuffer:
			{
				BufferDescription& bufferDesc = read.resource.buffer;
				ResourceId resourceId = namesToResourceIds[read.name];
				TransientBufferData& bufferData = transientResourceManager.GetTrackedBuffer(bufferDesc, trackedResources[resourceId].second);

				GraphicsAPI::AccessFlags srcAccess = bufferData.currentAccessFlags;
				GraphicsAPI::AccessFlags dstAccess = read.resource.resourceType == ResourceType::UniformBuffer
					? GraphicsAPI::AccessFlags::UNIFORMRead
					: GraphicsAPI::AccessFlags::ShaderRead;

				if (srcAccess != dstAccess) {
					bufferBarriers.emplace_back(
						GraphicsAPI::BufferBarrier{
							.buffer = bufferData.buffer,
							.srcAccess = srcAccess,
							.dstAccess = dstAccess,
							.offset = 0,
							.size = static_cast<uint32_t>(bufferDesc.size)
						}
					);

					bufferData.currentAccessFlags = dstAccess;
				}
				break;
			}
			}
		}

		for (ResourceReadWrite& readWrite : pass.readWrites) {
			switch (readWrite.resource.resourceType) {
			case ResourceType::ColorAttachment:
			case ResourceType::DepthAttachment:
			case ResourceType::StorageImage:
			{
				ImageDescription& imageDesc = readWrite.resource.image;
				ResourceId resourceId = namesToResourceIds[readWrite.input];
				TransientImageData& imageData = transientResourceManager.GetTrackedImage(context.swapchainSize.x, context.swapchainSize.y, imageDesc, trackedResources[resourceId].second);

				GraphicsAPI::ImageLayout srcLayout = imageData.currentLayout;
				GraphicsAPI::AccessFlags srcAccess = imageData.currentAccessFlags;
				GraphicsAPI::ImageLayout dstLayout = GraphicsAPI::ImageLayout::Undefined;
				GraphicsAPI::AccessFlags dstAccess = GraphicsAPI::AccessFlags::None;
				GraphicsAPI::PipelineStageBit dstPipelineStageBit = GraphicsAPI::PipelineStageBit::None;

				std::tie(dstLayout, dstAccess, dstPipelineStageBit) = InferBarrierData(pass.type, readWrite.resource.resourceType, AccessType::ReadWrite);

				if (srcLayout != dstLayout && srcAccess != dstAccess) {
					GraphicsAPI::ImageAspectBits aspect = GraphicsAPI::GetFormatAspect(imageDesc.format);
					imageBarriers.emplace_back(
						GraphicsAPI::ImageBarrier{
							.image = imageData.image,
							.oldLayout = srcLayout,
							.newLayout = dstLayout,
							.srcAccess = srcAccess,
							.dstAccess = dstAccess,
							.imageAspect = aspect,
							.baseMipLevel = 0,
							.levelCount = imageDesc.mipLevels,
							.baseArrayLayer = 0,
							.layerCount = imageDesc.arrayLayers
						}
					);

					dstPipelineStageMask |= dstPipelineStageBit;
					imageData.currentLayout = dstLayout;
					imageData.currentAccessFlags = dstAccess;
				}
				break;
			}
			case ResourceType::StorageBuffer:
			case ResourceType::UniformBuffer:
			{
				BufferDescription& bufferDesc = readWrite.resource.buffer;
				ResourceId resourceId = namesToResourceIds[readWrite.input];
				TransientBufferData& bufferData = transientResourceManager.GetTrackedBuffer(bufferDesc, trackedResources[resourceId].second);

				GraphicsAPI::AccessFlags srcAccess = bufferData.currentAccessFlags;
				GraphicsAPI::AccessFlags dstAccess = GraphicsAPI::AccessFlags::ShaderWrite;

				if (srcAccess != dstAccess) {
					bufferBarriers.emplace_back(
						GraphicsAPI::BufferBarrier{
							.buffer = bufferData.buffer,
							.srcAccess = srcAccess,
							.dstAccess = dstAccess,
							.offset = 0,
							.size = static_cast<uint32_t>(bufferDesc.size)
						}
					);

					bufferData.currentAccessFlags = dstAccess;
				}
				break;
			}
			}
		}

		for (ResourceWrite& write : pass.writes) {
			switch (write.resource.resourceType) {
			case ResourceType::ColorAttachment:
			case ResourceType::DepthAttachment:
			case ResourceType::StorageImage:
			{
				ImageDescription& imageDesc = write.resource.image;
				ResourceId resourceId = namesToResourceIds[write.name];
				TransientImageData& imageData = transientResourceManager.GetTrackedImage(context.swapchainSize.x, context.swapchainSize.y, imageDesc, trackedResources[resourceId].second);

				GraphicsAPI::ImageLayout srcLayout = imageData.currentLayout;
				GraphicsAPI::AccessFlags srcAccess = imageData.currentAccessFlags;
				GraphicsAPI::ImageLayout dstLayout = GraphicsAPI::ImageLayout::Undefined;
				GraphicsAPI::AccessFlags dstAccess = GraphicsAPI::AccessFlags::None;
				GraphicsAPI::PipelineStageBit dstPipelineStageBit = GraphicsAPI::PipelineStageBit::None;

				std::tie(dstLayout, dstAccess, dstPipelineStageBit) = InferBarrierData(pass.type, write.resource.resourceType, AccessType::Write);

				if (srcLayout != dstLayout && srcAccess != dstAccess) {
					GraphicsAPI::ImageAspectBits aspect = GraphicsAPI::GetFormatAspect(imageDesc.format);
					imageBarriers.emplace_back(
						GraphicsAPI::ImageBarrier{
							.image = imageData.image,
							.oldLayout = srcLayout,
							.newLayout = dstLayout,
							.srcAccess = srcAccess,
							.dstAccess = dstAccess,
							.imageAspect = aspect,
							.baseMipLevel = 0,
							.levelCount = imageDesc.mipLevels,
							.baseArrayLayer = 0,
							.layerCount = imageDesc.arrayLayers
						}
					);

					imageData.currentLayout = dstLayout;
					imageData.currentAccessFlags = dstAccess;
					dstPipelineStageMask |= dstPipelineStageBit;
				}
				break;
			}
			case ResourceType::StorageBuffer:
			case ResourceType::UniformBuffer:
			{
				BufferDescription& bufferDesc = write.resource.buffer;
				ResourceId resourceId = namesToResourceIds[write.name];
				TransientBufferData& bufferData = transientResourceManager.GetTrackedBuffer(bufferDesc, trackedResources[resourceId].second);

				GraphicsAPI::AccessFlags srcAccess = bufferData.currentAccessFlags;
				GraphicsAPI::AccessFlags dstAccess = GraphicsAPI::AccessFlags::ShaderWrite;

				if (srcAccess != dstAccess) {
					bufferBarriers.emplace_back(
						GraphicsAPI::BufferBarrier{
							.buffer = bufferData.buffer,
							.srcAccess = srcAccess,
							.dstAccess = dstAccess,
							.offset = 0,
							.size = static_cast<uint32_t>(bufferDesc.size)
						}
					);

					bufferData.currentAccessFlags = dstAccess;
				}
				break;
			}
			}
		}
	}
}
*/

static void RealizeResources() {
	/*
	for (size_t i = 0; i < resources.size(); ++i) {
		ResourceWrite& resource = resources[i];
		void* trackedResource = nullptr;
		UnionResource& unionResource = resource.resource;
		switch (unionResource.resourceType) {
		case ResourceType::StorageBuffer:
		case ResourceType::UniformBuffer: {
			Renderer::TransientBufferData bufferData;
			size_t index;
			std::tie(bufferData, index) = transientResourceManager.AddTrackedBuffer(resource.resource.buffer);
			trackedResources.emplace_back(
				TransientResourceUnion{
					.buffer = bufferData
				},
				index
			);
			break;
		}
		case ResourceType::ColorAttachment:
		case ResourceType::DepthAttachment:
		case ResourceType::StorageImage: {
			Renderer::TransientImageData imageData;
			size_t index;
			std::tie(imageData, index) = transientResourceManager.AddTrackedImage(context.swapchainSize.x, context.swapchainSize.y, resource.resource.image);
			trackedResources.emplace_back(
				TransientResourceUnion{
					.image = imageData
				},
				index
			);
			break;
		}
		default:
			GS_ASSERT_LOG("Invalid ResourceType!");
		}
	}
	*/
}
