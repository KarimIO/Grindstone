#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>

// ===============================================================
// - RenderPassExecution
// ===============================================================

Grindstone::GraphicsAPI::DescriptorSet* Grindstone::Renderer::RenderGraph::RenderPassExecution::GetPassDescriptorSet() const {
	return descriptorSet;
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

		if (!imageBarriers.empty() || !bufferBarriers.empty()) {
			cmd->PipelineBarrier(
				bufferBarriers.data(), static_cast<uint32_t>(bufferBarriers.size()),
				imageBarriers.data(), static_cast<uint32_t>(imageBarriers.size())
			);
		}

		const char* passName = pass.name.ToString().c_str();
		Grindstone::GraphicsAPI::DescriptorSet* descriptorSet = MakeDescriptors(pass);

		pass.Execute(context);
	}

	for (auto& outputResource : outputResources) {
		ResourceId resourceId = namesToResourceIds[outputResource.first];
		RenderGraph::ResourceWrite& resource = resources[resourceId];
		switch (resource.resource.resourceType) {
		case ResourceType::ColorAttachment:
		case ResourceType::DepthAttachment:
		case ResourceType::StorageImage:
			outputResource.second.image = transientResourceManager.GetTrackedImage(context.swapchainSize.x, context.swapchainSize.y, resource.resource.image, trackedResources[resourceId].second);
			break;
		case ResourceType::StorageBuffer:
		case ResourceType::UniformBuffer:
			outputResource.second.buffer = transientResourceManager.GetTrackedBuffer(resource.resource.buffer, trackedResources[resourceId].second);
			break;
		}

		passes.clear();
	}
}
