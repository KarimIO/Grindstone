#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>

const int8_t USED_LIFETIME = 18;

void Grindstone::Renderer::TransientResourceManager::StartFrame() {
	Grindstone::GraphicsAPI::Core* graphicsCore = Grindstone::EngineCore::GetInstance().GetGraphicsCore();

	for (auto& imageSet : images) {
		auto& transientImagesArray = imageSet.second;

		auto iter = transientImagesArray.begin();
		while (iter != transientImagesArray.end()) {
			if (iter->lifetime == 0) {
				graphicsCore->DeleteImage(iter->data.image);
				iter = transientImagesArray.erase(iter);
			}
			else {
				iter->isUsedThisFrame = false;
				++iter;
			}
		}
	}

	for (auto& bufferSet : buffers) {
		auto& transientBuffersArray = bufferSet.second;
		auto iter = transientBuffersArray.begin();
		while (iter != transientBuffersArray.end()) {
			if (iter->lifetime == 0) {
				graphicsCore->DeleteBuffer(iter->data.buffer);
				iter = transientBuffersArray.erase(iter);
			}
			else {
				iter->isUsedThisFrame = false;
				++iter;
			}
		}
	}

	std::erase_if(images, [](auto& kv) { return kv.second.empty(); });
	std::erase_if(buffers, [](auto& kv) { return kv.second.empty(); });
}

std::tuple<Grindstone::Renderer::TransientImageData, size_t> Grindstone::Renderer::TransientResourceManager::AddTrackedImage(uint32_t viewportWidth, uint32_t viewportHeight, ImageDescription inDesc) {
	uint32_t width;
	uint32_t height;

	if (inDesc.sizeClass == Grindstone::Renderer::ImageSizeType::SwapchainRelative) {
		width = static_cast<uint32_t>(static_cast<float>(viewportWidth) * inDesc.width + 0.5f);
		height = static_cast<uint32_t>(static_cast<float>(viewportHeight) * inDesc.height + 0.5f);
	}
	else {
		width = static_cast<uint32_t>(inDesc.width + 0.5f);
		height = static_cast<uint32_t>(inDesc.height + 0.5f);
	}

	TransientImageDescription desc{
		.width = width,
		.height = height,
		.samples = inDesc.samples,
		.mipLevels = inDesc.mipLevels,
		.depth = inDesc.depth,
		.arrayLayers = inDesc.arrayLayers,
		.format = inDesc.format,

		.imageDimensions = inDesc.imageDimensions,
		.memoryUsage = inDesc.memoryUsage,
		.imageUsage = inDesc.imageUsage
	};

	auto it = images.find(desc);
	if (it != images.end()) {
		auto& arr = it->second;
		for (size_t index = 0; index < arr.size(); ++index) {
			auto& img = it->second[index];
			if (!img.isUsedThisFrame) {
				img.lifetime = USED_LIFETIME;
				img.isUsedThisFrame = true;
				return { img.data, index };
			}
		}
	}

	Grindstone::GraphicsAPI::Image::CreateInfo createInfo{
		.debugName = "TRACKED Image",
		.width = width,
		.height = height,
		.depth = desc.depth,
		.mipLevels = desc.mipLevels,
		.arrayLayers = desc.arrayLayers,

		.format = desc.format,
		.imageDimensions = desc.imageDimensions,
		.memoryUsage = desc.memoryUsage,
		.imageUsage = desc.imageUsage
	};

	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	GS_ASSERT(graphicsCore != nullptr);

	GraphicsAPI::Image* image = graphicsCore->CreateImage(createInfo);
	GS_ASSERT(image != nullptr);

	auto& imageTypeEntry = images[desc];
	size_t index = imageTypeEntry.size();
	auto& newImage = imageTypeEntry.emplace_back(
		TransientImage{
			.isUsedThisFrame = true,
			.lifetime = USED_LIFETIME,
			.data = TransientImageData {
				.image = image,
				.currentLayout = GraphicsAPI::ImageLayout::Undefined,
				.currentAccessFlags = GraphicsAPI::AccessFlags::None
			}
		}
	);

	return { newImage.data, index };
}

std::tuple<Grindstone::Renderer::TransientBufferData, size_t> Grindstone::Renderer::TransientResourceManager::AddTrackedBuffer(BufferDescription desc) {
	auto it = buffers.find(desc);
	if (it != buffers.end()) {
		auto& arr = it->second;
		for (size_t index = 0; index < arr.size(); ++index) {
			auto& b = it->second[index];
			if (!b.isUsedThisFrame) {
				b.lifetime = USED_LIFETIME;
				b.isUsedThisFrame = true;
				return { b.data, index };
			}
		}
	}

	Grindstone::GraphicsAPI::Buffer::CreateInfo createInfo{
		.debugName = "TRACKED Buffer", // TODO: How to handle debug names
		.content = nullptr,
		.bufferSize = desc.size,
		.bufferUsage = desc.bufferUsage,
		.memoryUsage = desc.memoryUsage
	};

	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	GS_ASSERT(graphicsCore != nullptr);

	GraphicsAPI::Buffer* buffer = graphicsCore->CreateBuffer(createInfo);
	GS_ASSERT(buffer != nullptr);

	auto& bufferTypeEntry = buffers[desc];
	size_t index = bufferTypeEntry.size();
	auto& bufferData = bufferTypeEntry.emplace_back(
		TransientBuffer{
			.isUsedThisFrame = true,
			.lifetime = USED_LIFETIME,
			.data = TransientBufferData {
				.buffer = buffer,
				.currentAccessFlags = GraphicsAPI::AccessFlags::None
			}
		}
	);

	return { bufferData.data, index };
}

Grindstone::Renderer::TransientImageData& Grindstone::Renderer::TransientResourceManager::GetTrackedImage(uint32_t viewportWidth, uint32_t viewportHeight, ImageDescription inDesc, size_t index) {
	uint32_t width;
	uint32_t height;

	if (inDesc.sizeClass == Grindstone::Renderer::ImageSizeType::SwapchainRelative) {
		width = static_cast<uint32_t>(static_cast<float>(viewportWidth) * inDesc.width + 0.5f);
		height = static_cast<uint32_t>(static_cast<float>(viewportHeight) * inDesc.height + 0.5f);
	}
	else {
		width = static_cast<uint32_t>(inDesc.width + 0.5f);
		height = static_cast<uint32_t>(inDesc.height + 0.5f);
	}

	TransientImageDescription desc{
		.width = width,
		.height = height,
		.samples = inDesc.samples,
		.mipLevels = inDesc.mipLevels,
		.depth = inDesc.depth,
		.arrayLayers = inDesc.arrayLayers,
		.format = inDesc.format,

		.imageDimensions = inDesc.imageDimensions,
		.memoryUsage = inDesc.memoryUsage,
		.imageUsage = inDesc.imageUsage
	};

	auto it = images.find(desc);
	GS_ASSERT(it != images.end());

	return it->second[index].data;
}

Grindstone::Renderer::TransientBufferData& Grindstone::Renderer::TransientResourceManager::GetTrackedBuffer(BufferDescription desc, size_t index) {
	auto it = buffers.find(desc);
	GS_ASSERT(it != buffers.end());

	return it->second[index].data;
}

// ===============================================================
// - RenderGraph Pass
// ===============================================================

// Storage

void Grindstone::Renderer::RenderGraph::RenderPass::ReadStorageImage(Grindstone::HashedString inputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Compute);
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::StorageImage, resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::ReadWriteStorageImage(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Compute);
	readWrites.emplace_back(ResourceReadWrite(inputName, inputName, UnionResource(ResourceType::StorageImage, resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::WriteStorageImage(Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Compute);
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::StorageImage, resource)));
}

// Color

void Grindstone::Renderer::RenderGraph::RenderPass::ReadColorAttachment(Grindstone::HashedString inputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::ReadWriteColorAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	readWrites.emplace_back(ResourceReadWrite(inputName, inputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::WriteColorAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearColor clearValue) {
	GS_ASSERT(type == GpuQueue::Graphics);
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::ColorAttachment, resource)));
}

// Depth Stencil

void Grindstone::Renderer::RenderGraph::RenderPass::ReadDepthStencilAttachment(Grindstone::HashedString inputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	reads.emplace_back(ResourceRead(inputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::ReadWriteDepthStencilAttachment(Grindstone::HashedString inputName, Grindstone::HashedString outputName, ImageDescription resource) {
	GS_ASSERT(type == GpuQueue::Graphics);
	readWrites.emplace_back(ResourceReadWrite(inputName, outputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::WriteDepthStencilAttachment(Grindstone::HashedString outputName, ImageDescription resource, Grindstone::GraphicsAPI::ClearDepthStencil clearValue) {
	GS_ASSERT(type == GpuQueue::Graphics);
	writes.emplace_back(ResourceWrite(outputName, UnionResource(ResourceType::DepthAttachment, resource)));
}

// Buffers

void Grindstone::Renderer::RenderGraph::RenderPass::ReadBuffer(Grindstone::HashedString inputName, BufferDescription resource) {
	reads.emplace_back(ResourceRead(inputName, UnionResource(resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::ReadWriteBuffer(Grindstone::HashedString inputName, Grindstone::HashedString outputName, BufferDescription resource) {
	readWrites.emplace_back(ResourceReadWrite(inputName, outputName, UnionResource(resource)));
}

void Grindstone::Renderer::RenderGraph::RenderPass::WriteBuffer(Grindstone::HashedString outputName, BufferDescription resource) {
	writes.emplace_back(ResourceWrite(outputName, UnionResource(resource)));
}

// ===============================================================
// - RenderPassExecution
// ===============================================================

Grindstone::GraphicsAPI::DescriptorSet* Grindstone::Renderer::RenderGraph::RenderPassExecution::GetPassDescriptorSet() const {
	return descriptorSet;
}

// ===============================================================
// - RenderGraph
// ===============================================================

void Grindstone::Renderer::RenderGraph::AddComputePass(
	Grindstone::HashedString name,
	std::function<void(RenderPass&)> setup,
	std::function<void(RenderGraphContext&, RenderPassExecution&)> exec
) {
	Grindstone::Renderer::RenderGraph::RenderPass& renderPass = passes.emplace_back(Grindstone::Renderer::RenderGraph::RenderPass());
	renderPass.name = name;
	renderPass.type = GpuQueue::Compute;
	renderPass.execution = exec;
	setup(renderPass);
}

void Grindstone::Renderer::RenderGraph::AddGraphicsPass(
	Grindstone::HashedString name,
	std::function<void(RenderPass&)> setup,
	std::function<void(RenderGraphContext&, RenderPassExecution&)> exec
) {
	Grindstone::Renderer::RenderGraph::RenderPass& renderPass = passes.emplace_back(Grindstone::Renderer::RenderGraph::RenderPass());
	renderPass.name = name;
	renderPass.type = GpuQueue::Graphics;
	renderPass.execution = exec;
	setup(renderPass);
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

	// Kahn’s Algorithm (BFS)
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

void Grindstone::Renderer::RenderGraph::SortPasses() {
	const uint32_t numPasses = static_cast<uint32_t>(passes.size());

	std::map<Grindstone::HashedString, uint32_t> dependencySources;
	std::map<Grindstone::HashedString, UnionResource> dependencyDefinition;

	// Prepare for topological sort.
	std::vector<std::vector<int>> passesToSort;
	passesToSort.resize(numPasses);
	for (size_t i = 0; i < numPasses; ++i) {
		Grindstone::Renderer::RenderGraph::RenderPass& pass = passes[i];

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
		Grindstone::Renderer::RenderGraph::RenderPass& pass = passes[i];
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
	// sortedPasses.reserve(numPasses);
	// for (uint32_t i = 0; i < numPasses; i++) {
	// 	sortedPasses.push_back(passes[sortedIndices[i]]);
	// }
}

void Grindstone::Renderer::RenderGraph::SetupDependencies(
	std::vector<ResourceWrite>& resources,
	std::map<Grindstone::HashedString, ResourceId>& namesToResourceIds
) {
	// Create resources in Writes
	std::queue<Grindstone::Renderer::RenderGraph::RenderPass*> passesWithReads;
	for (Grindstone::Renderer::RenderGraph::RenderPass& pass : passes) {
		for(RenderGraph::ResourceWrite& resource : pass.writes) {
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
		Grindstone::Renderer::RenderGraph::RenderPass& pass = *passesWithReads.front();
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

static Grindstone::GraphicsAPI::DescriptorSet* MakeDescriptors(Grindstone::Renderer::RenderGraph::RenderPass& pass) {
	Grindstone::GraphicsAPI::Core* graphicsCore = Grindstone::EngineCore::GetInstance().GetGraphicsCore();

	const char* passName = pass.name.ToString().c_str();

	std::string descriptorSetLayoutDebugName = std::vformat("{} Descriptor Set Layout", std::make_format_args(passName));
	std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> layoutBindings;
	std::vector<Grindstone::GraphicsAPI::DescriptorSet::Binding> bindings;

	Grindstone::GraphicsAPI::ShaderStageBit stages = pass.type == Grindstone::Renderer::GpuQueue::Compute
		? Grindstone::GraphicsAPI::ShaderStageBit::Compute
		: Grindstone::GraphicsAPI::ShaderStageBit::Vertex | Grindstone::GraphicsAPI::ShaderStageBit::Fragment;

	for (size_t i = 0; i < pass.reads.size(); ++i) {
		Grindstone::Renderer::RenderGraph::ResourceRead& read = pass.reads[i];
		Grindstone::GraphicsAPI::DescriptorSetLayout::Binding layoutBinding{
			.bindingId = static_cast<uint32_t>(i),
			.count = 1,
			.stages = stages
		};

		Grindstone::GraphicsAPI::BindingType bindingType;
		void* resourcePtr = nullptr;

		switch (read.resource.resourceType) {
		case Grindstone::Renderer::RenderGraph::ResourceType::UniformBuffer:
			bindingType = Grindstone::GraphicsAPI::BindingType::UniformBuffer;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::StorageBuffer:
			bindingType = Grindstone::GraphicsAPI::BindingType::StorageBuffer;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::ColorAttachment:
			bindingType = Grindstone::GraphicsAPI::BindingType::CombinedImageSampler;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::DepthAttachment:
			bindingType = Grindstone::GraphicsAPI::BindingType::CombinedImageSampler;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::StorageImage:
			bindingType = Grindstone::GraphicsAPI::BindingType::StorageImage;
			break;
		}

		layoutBinding.type = bindingType;
		Grindstone::GraphicsAPI::DescriptorSet::Binding binding(resourcePtr, bindingType);

		layoutBindings.emplace_back(layoutBinding);
		bindings.emplace_back(binding);
	}

	for (size_t i = 0; i < pass.writes.size(); ++i) {
		Grindstone::Renderer::RenderGraph::ResourceWrite& write = pass.writes[i];
		Grindstone::GraphicsAPI::DescriptorSetLayout::Binding layoutBinding{
			.bindingId = static_cast<uint32_t>(i),
			.count = 1,
			.stages = stages
		};

		Grindstone::GraphicsAPI::BindingType bindingType;
		void* resourcePtr = nullptr;
		bool shouldAdd = true;

		switch (write.resource.resourceType) {
		case Grindstone::Renderer::RenderGraph::ResourceType::UniformBuffer:
			bindingType = Grindstone::GraphicsAPI::BindingType::UniformBuffer;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::StorageBuffer:
			bindingType = Grindstone::GraphicsAPI::BindingType::StorageBuffer;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::StorageImage:
			bindingType = Grindstone::GraphicsAPI::BindingType::StorageImage;
			break;
		default:
			shouldAdd = false;
			break;
		}

		if (shouldAdd) {
			layoutBinding.type = bindingType;
			Grindstone::GraphicsAPI::DescriptorSet::Binding binding(resourcePtr, bindingType);
			layoutBindings.emplace_back(layoutBinding);
			bindings.emplace_back(binding);
		}
	}

	for (size_t i = 0; i < pass.readWrites.size(); ++i) {
		Grindstone::Renderer::RenderGraph::ResourceReadWrite& readWrite = pass.readWrites[i];
		Grindstone::GraphicsAPI::DescriptorSetLayout::Binding layoutBinding{
			.bindingId = static_cast<uint32_t>(i),
			.count = 1,
			.stages = stages
		};

		Grindstone::GraphicsAPI::BindingType bindingType;
		void* resourcePtr = nullptr;

		bool shouldAdd = true;

		switch (readWrite.resource.resourceType) {
		case Grindstone::Renderer::RenderGraph::ResourceType::UniformBuffer:
			bindingType = Grindstone::GraphicsAPI::BindingType::UniformBuffer;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::StorageBuffer:
			bindingType = Grindstone::GraphicsAPI::BindingType::StorageBuffer;
			break;
		case Grindstone::Renderer::RenderGraph::ResourceType::StorageImage:
			bindingType = Grindstone::GraphicsAPI::BindingType::StorageImage;
			break;
		default:
			shouldAdd = false;
		}

		if (shouldAdd) {
			layoutBinding.type = bindingType;
			Grindstone::GraphicsAPI::DescriptorSet::Binding binding(resourcePtr, bindingType);
			layoutBindings.emplace_back(layoutBinding);
		}
	}

	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{
		.debugName = descriptorSetLayoutDebugName.c_str(),
		.bindings = layoutBindings.data(),
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
	};

	// TODO: Cache Descriptor Set Layouts
	Grindstone::GraphicsAPI::DescriptorSetLayout* layout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	std::string descriptorSetDebugName = std::vformat("{} Descriptor Set Layout", std::make_format_args(passName));

	Grindstone::GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{
		.debugName = descriptorSetDebugName.c_str(),
		.layout = layout,
		.bindings = bindings.data(),
		.bindingCount = static_cast<uint32_t>(bindings.size()),
	};

	// TODO: Pool Descriptor Sets
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);

	return descriptorSet;
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
	Grindstone::Renderer::GpuQueue gpuQueue,
	Grindstone::Renderer::RenderGraph::ResourceType resourceType,
	AccessType accessType
	// Grindstone::GraphicsAPI::ShaderStageBit stages
) {
	using namespace Grindstone::GraphicsAPI;
	using namespace Grindstone::Renderer;

	if (accessType == AccessType::Read) {
		return {
			ImageLayout::ShaderRead,
			AccessFlags::ShaderRead,
			gpuQueue == GpuQueue::Compute
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

void Grindstone::Renderer::RenderGraph::ExecuteGraph(Grindstone::Renderer::RenderGraph::RenderGraphContext context) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::GraphicsAPI::CommandBuffer* cmd = context.commandBuffer;

	std::vector<ResourceWrite> resources;
	std::map<Grindstone::HashedString, ResourceId> namesToResourceIds;
	std::vector<std::pair<TransientResourceUnion, size_t>> trackedResources;

	transientResourceManager.StartFrame();
	SortPasses();
	SetupDependencies(resources, namesToResourceIds);

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

	for (Grindstone::Renderer::RenderGraph::RenderPass& pass : passes) {
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

		if (!imageBarriers.empty() || !bufferBarriers.empty()) {
			cmd->PipelineBarrier(
				srcPipelineStageMask,
				dstPipelineStageMask,
				bufferBarriers.data(), static_cast<uint32_t>(bufferBarriers.size()),
				imageBarriers.data(), static_cast<uint32_t>(imageBarriers.size())
			);
		}

		const char* passName = pass.name.ToString().c_str();
		Grindstone::GraphicsAPI::DescriptorSet* descriptorSet = MakeDescriptors(pass);

		Grindstone::Renderer::RenderGraph::RenderPassExecution renderGraphExecution;
		renderGraphExecution.descriptorSet = descriptorSet;

		switch (pass.type) {
		case GpuQueue::Graphics: {
			std::vector<Grindstone::GraphicsAPI::RenderAttachment> colorAttachments;
			Grindstone::GraphicsAPI::RenderAttachment depthAttachment;
			uint32_t depthAttachments = 0; // Should be one

			for (RenderGraph::ResourceReadWrite& readWrite : pass.readWrites) {
				ResourceId resourceId = namesToResourceIds[readWrite.output];
				TransientImageData& imageData = transientResourceManager.GetTrackedImage(context.swapchainSize.x, context.swapchainSize.y, readWrite.resource.image, trackedResources[resourceId].second);
				Grindstone::GraphicsAPI::Image* image = static_cast<Grindstone::GraphicsAPI::Image*>(imageData.image);

				if (readWrite.resource.resourceType == ResourceType::ColorAttachment) {
					Grindstone::GraphicsAPI::ImageLayout imageLayout = imageData.currentLayout;
					colorAttachments.emplace_back(
						Grindstone::GraphicsAPI::RenderAttachment{
							.image = image,
							.imageLayout = imageLayout,
							.loadOp = Grindstone::GraphicsAPI::LoadOp::Load,
							.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
						}
					);
				}
				else if (readWrite.resource.resourceType == ResourceType::DepthAttachment) {
					Grindstone::GraphicsAPI::ImageLayout imageLayout = imageData.currentLayout;
					depthAttachment = Grindstone::GraphicsAPI::RenderAttachment{
						.image = image,
						.imageLayout = imageLayout,
						.loadOp = Grindstone::GraphicsAPI::LoadOp::Load,
						.storeOp = Grindstone::GraphicsAPI::StoreOp::Store
					};
					++depthAttachments;
				}
			}

			for (RenderGraph::ResourceWrite& write : pass.writes) {
				ResourceId resourceId = namesToResourceIds[write.name];
				TransientImageData& imageData = transientResourceManager.GetTrackedImage(context.swapchainSize.x, context.swapchainSize.y, write.resource.image, trackedResources[resourceId].second);

				Grindstone::GraphicsAPI::Image* image = static_cast<Grindstone::GraphicsAPI::Image*>(imageData.image);
				if (write.resource.resourceType == ResourceType::ColorAttachment) {
					Grindstone::GraphicsAPI::ClearUnion clearValue = GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					Grindstone::GraphicsAPI::ImageLayout imageLayout = imageData.currentLayout;
					colorAttachments.emplace_back(
						Grindstone::GraphicsAPI::RenderAttachment{
							.image = image,
							.imageLayout = imageLayout,
							.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
							.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
							.clearValue = clearValue
						}
					);
				}
				else if (write.resource.resourceType == ResourceType::DepthAttachment) {
					Grindstone::GraphicsAPI::ClearUnion clearValue = GraphicsAPI::ClearDepthStencil{ .depth = 1.0f, .stencil = 0 };
					Grindstone::GraphicsAPI::ImageLayout imageLayout = imageData.currentLayout;
					depthAttachment = Grindstone::GraphicsAPI::RenderAttachment{
						.image = image,
						.imageLayout = imageLayout,
						.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
						.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
						.clearValue = clearValue
					};
					++depthAttachments;
				}
			}

			GS_ASSERT(depthAttachments <= 1);

			cmd->BeginRendering(
				passName,
				context.swapchainSize,
				colorAttachments.data(),
				colorAttachments.size(),
				depthAttachments >= 1 ? &depthAttachment : nullptr
			);

			pass.execution(context, renderGraphExecution);

			cmd->EndRendering();
			break;
		}
		case GpuQueue::Compute: {
			pass.execution(context, renderGraphExecution);
			break;
		}
		default:
			GS_ASSERT_LOG("Invalid pass type!");
			break;
		}

		// TODO: Pool Descriptor Sets
		engineCore.PushDeletion(
			[descriptorSet] {
				Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
				Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
				graphicsCore->DeleteDescriptorSet(descriptorSet);
			}
		);
	}

	passes.clear();
}
