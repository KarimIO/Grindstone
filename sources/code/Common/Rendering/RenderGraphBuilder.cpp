#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>

#include "RenderGraphBuilder.hpp"

// ===============================================================
// - Setup Phase
// ===============================================================

// TODO: These should be allocted on a lienar allocator.

Grindstone::Renderer::TransferRenderGraphBuilderPass* Grindstone::Renderer::RenderGraphBuilder::CreateTransferPass(
	Grindstone::StringRef name,
	std::function<void(Grindstone::Renderer::TransferRenderGraphBuilderPass&)> setupImmediateCallback
) {
	auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<TransferRenderGraphBuilderPass>());
	auto pass = static_cast<TransferRenderGraphBuilderPass*>(uniquePtr.Get());
	pass->name = name;
	setupImmediateCallback(*pass);
	return pass;
}

Grindstone::Renderer::PresentRenderGraphBuilderPass* Grindstone::Renderer::RenderGraphBuilder::CreatePresentPass(TGBImageRef imageRef) {
	auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<PresentRenderGraphBuilderPass>());
	auto pass = static_cast<PresentRenderGraphBuilderPass*>(uniquePtr.Get());
	pass->name = "Present to Screen";
	pass->SetPresentationImage(imageRef);
	return pass;
}

void Grindstone::Renderer::RenderGraphBuilder::CreatePresentPass(
	std::function<void(Grindstone::Renderer::PresentRenderGraphBuilderPass&)> setupImmediateCallback
) {
	auto& uniquePtr = passes.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<Grindstone::Renderer::PresentRenderGraphBuilderPass>());
	auto pass = static_cast<PresentRenderGraphBuilderPass*>(uniquePtr.Get());
	pass->name = "Present to Screen";
	setupImmediateCallback(*pass);
}

// ===============================================================
// - Compile Phase
// ===============================================================

static void SetupBarriers();
static std::set<Grindstone::Renderer::PassId> CullUnusedPasses(
	std::vector<Grindstone::Renderer::ResourceId>& externalOutputs,
	std::vector<std::vector<Grindstone::Renderer::ResourceId>>& passDependencies,
	std::vector<Grindstone::Renderer::PassId>& resourceSourcePasses,
	Grindstone::Renderer::ResourceId presentationResource
);
static void SortPasses();
static void SetupResourceGraph(
	std::vector<Grindstone::UniquePtr<Grindstone::Renderer::RenderGraphBuilderPass>>& passes,
	std::vector<ResourceWrite>& resources,
	std::map<Grindstone::HashedString, ResourceId>& namesToResourceIds
);
static void RealizeResources();
static void SetupAttachments();

Grindstone::Renderer::RenderGraph Grindstone::Renderer::RenderGraphBuilder::Compile() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	std::vector<ResourceWrite> resources;
	std::map<Grindstone::HashedString, ResourceId> resourceNamesToPassIndices;
	std::vector<std::pair<TransientResourceUnion, size_t>> trackedResources;

	SetupResourceGraph(passes);
	std::set<PassId> remainingPasses = CullUnusedPasses();
	SortPasses();
	SetupBarriers();
	RealizeResources();
	SetupAttachments();

	passes.clear();

	return Grindstone::Renderer::RenderGraph();
}

template<typename IdType>
static std::vector<IdType> TopologicalSortKahn(std::vector<std::vector<IdType>>& graph) {
	IdType n = static_cast<IdType>(graph.size());
	std::vector<IdType> indegree(n, 0);
	std::vector<IdType> list;

	// Compute indegrees (number of dependencies) for each pass.
	for (IdType i = 0; i < n; i++) {
		for (IdType next : graph[i]) {
			indegree[next]++;
		}
	}

	std::queue<IdType> queue;

	// Add all nodes with indegree 0 into the front of the queue
	// because they should be as early as possible.
	for (IdType i = 0; i < n; i++) {
		if (indegree[i] == 0) {
			queue.push(i);
		}
	}

	// Kahn's Algorithm (BFS)
	while (!queue.empty()) {
		IdType top = queue.front();
		queue.pop();
		list.push_back(top);
		for (IdType next : graph[top]) {
			indegree[next]--;
			if (indegree[next] == 0) {
				queue.push(next);
			}
		}
	}

	return list;
}

static std::set<Grindstone::Renderer::PassId> CullUnusedPasses(
	std::vector<Grindstone::Renderer::ResourceId>& externalOutputs,
	std::vector<std::vector<Grindstone::Renderer::ResourceId>>& passDependencies,
	std::vector<Grindstone::Renderer::PassId>& resourceSourcePasses,
	Grindstone::Renderer::ResourceId presentationResource
) {
	using namespace Grindstone::Renderer;

	std::set<PassId> usedPasses;
	std::queue<ResourceId> resourcesToProcess;

	// Place external output resources in the queue, so we can begin tracing which passes are required to compute them.
	for (ResourceId resourceId : externalOutputs) {
		resourcesToProcess.push(resourceId);
	}

	// Also add the present resource, which is also considered essential so we can compute which passes are required.
	if (presentationResource != invalidResourceId) {
		resourcesToProcess.push(presentationResource);
	}

	// For each resource in resources queue:
	while (!resourcesToProcess.empty()) {
		ResourceId top = resourcesToProcess.front();
		resourcesToProcess.pop();

		PassId resourceSourcePass = resourceSourcePasses[top];

		// Mark this resource's source pass as used.
		usedPasses.insert(resourceSourcePass);

		std::vector<ResourceId>& passInputs = passDependencies[resourceSourcePass];
		for (ResourceId passInput : passInputs) {
			// NOTE: This works as long as we don't have ciruclar dependencies.
			// - maybe we should check if this resource has already been evaluated?
			resourcesToProcess.push(passInput);
		}
	}

	return usedPasses;
}

static void SortPasses() {
	const uint32_t numPasses = static_cast<uint32_t>(passes.size());

	std::map<Grindstone::HashedString, uint32_t> dependencySources;
	std::map<Grindstone::HashedString, UnionResource> dependencyDefinition;

	// Prepare for topological sort.
	std::vector<std::vector<int>> passesToSort;
	passesToSort.resize(numPasses);
	for (size_t i = 0; i < numPasses; ++i) {
		Grindstone::Renderer::RenderGraphBuilderPass& pass = passes[i];

		for (ResourceWrite& resource : pass.writes) {
			dependencySources[resource.name] = i;
			dependencyDefinition[resource.name] = resource.resource;
		}

		for (ResourceReadWrite& resource : pass.readWrites) {
			dependencySources[resource.output] = i;
			dependencyDefinition[resource.output] = resource.resource;
		}
	}

	for (size_t i = 0; i < numPasses; ++i) {
		Grindstone::Renderer::RenderGraphBuilderPass& pass = passes[i];
		if (pass.reads.empty() && pass.readWrites.empty()) {
			continue;
		}

		std::vector<int>& deps = passesToSort.emplace_back();
		for (ResourceWrite& resource : pass.writes) {
			GS_ASSERT(dependencyDefinition[resource.name] == resource.resource);

			auto it = dependencySources.find(resource.name);
			GS_ASSERT(it != dependencySources.end());
			deps.emplace_back(it->second);
		}

		for (ResourceReadWrite& resource : pass.readWrites) {
			GS_ASSERT(dependencyDefinition[resource.input] == resource.resource);

			auto it = dependencySources.find(resource.input);
			GS_ASSERT(it != dependencySources.end());
			deps.emplace_back(it->second);
		}
	}

	// Do topological sort.
	auto sortedIndices = TopologicalSortKahn(passesToSort);

	// Add the sorted passes to the Graph.
	sortedPasses.reserve(numPasses);
	for (uint32_t i = 0; i < numPasses; i++) {
		sortedPasses.push_back(passes[sortedIndices[i]]);
	}
}

static void Grindstone::Renderer::RenderGraphBuilder::SetupDependencies(
	std::vector<ResourceWrite>& resources,
	std::map<Grindstone::HashedString, ResourceId>& namesToResourceIds
) {
	// Create resources in Writes
	std::queue<Grindstone::Renderer::RenderGraphBuilderPass*> passesWithReads;
	for (Grindstone::Renderer::RenderGraphBuilderPass& pass : passes) {
		for (RenderGraph::ResourceWrite& resource : pass.writes) {
			RenderGraph::ResourceId resourceId = resources.size();
			GS_ASSERT(namesToResourceIds.find(resource.name) == namesToResourceIds.end()); // Resource Writes should be unique!
			namesToResourceIds[resource.name] = resourceId;
			resources.emplace_back(resource.name, resource.resource);
		}

		if (pass.reads.size() > 0 || pass.readWrites.size() > 0) {
			passesWithReads.push(&pass);
		}
	}

	// Try to link ReadWrites to original Writes
	while (!passesWithReads.empty()) {
		Grindstone::Renderer::RenderGraphBuilderPass& pass = *passesWithReads.front();
		passesWithReads.pop();

		bool isMissingResource = false;
		for (RenderGraph::ResourceReadWrite& resource : pass.readWrites) {
			auto iterator = namesToResourceIds.find(resource.input);
			if (iterator != namesToResourceIds.end()) {
				namesToResourceIds[resource.output] = iterator->second;
			}
			else {
				isMissingResource = true;
			}
		}

		if (isMissingResource) {
			passesWithReads.push(&pass);
		}
	}
}

enum class AccessType {
	Read,
	Write,
	ReadWrite
};

static std::tuple<
	Grindstone::GraphicsAPI::ImageLayout,
	Grindstone::GraphicsAPI::AccessFlags,
	Grindstone::GraphicsAPI::PipelineStageBit
> InferBarrierData(
	Grindstone::Renderer::GpuPassType gpuQueue,
	Grindstone::Renderer::RenderGraphBuilder::ResourceType resourceType,
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
	for (Grindstone::Renderer::RenderGraphPass& pass : passes) {
		GraphicsAPI::PipelineStageBit srcPipelineStageMask = GraphicsAPI::PipelineStageBit::AllGraphics;
		GraphicsAPI::PipelineStageBit dstPipelineStageMask = GraphicsAPI::PipelineStageBit::None;
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

static void RealizeResources() {
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
}
