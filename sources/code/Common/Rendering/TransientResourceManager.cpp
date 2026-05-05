#include <Common/Graphics/Core.hpp>

#include <EngineCore/EngineCore.hpp>

#include "TransientResourceManager.hpp"

const int8_t USED_LIFETIME = 18;

static Grindstone::Renderer::TransientImageDescription ToTransient(
	Grindstone::Renderer::ImageDescription desc,
	Grindstone::Math::Uint2 viewportResolution,
	Grindstone::Math::Uint2 swapchainResolution
) {
	uint32_t width = desc.size.x.Resolve(viewportResolution.x, swapchainResolution.x);
	uint32_t height = desc.size.y.Resolve(viewportResolution.y, swapchainResolution.y);

	return {
		.size = Grindstone::Math::Uint2(width, height),
		.samples = desc.samples,
		.mipLevels = desc.mipLevels,
		.depth = desc.depth,
		.arrayLayers = desc.arrayLayers,
		.format = desc.format,

		.imageDimensions = desc.imageDimensions,
		.memoryUsage = desc.memoryUsage,
		.imageUsage = desc.imageUsage
	};
}

static Grindstone::Renderer::TransientBufferDescription ToTransient(
	Grindstone::Renderer::BufferDescription desc
) {
	return {
		.size = desc.size,
		.bufferUsage = desc.bufferUsage,
		.memoryUsage = desc.memoryUsage
	};
}

void Grindstone::Renderer::TransientResourceManager::BeginFrame() {
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

Grindstone::Renderer::TransientImageKey Grindstone::Renderer::TransientResourceManager::AcquireImage(
	Math::Uint2 viewportResolution,
	Math::Uint2 swapchainResolution,
	const Grindstone::Renderer::ImageDescription& inDesc
) {
	TransientImageDescription desc = ToTransient(inDesc, viewportResolution, swapchainResolution);

	auto it = images.find(desc);
	if (it != images.end()) {
		auto& arr = it->second;
		for (size_t index = 0; index < arr.size(); ++index) {
			auto& img = it->second[index];
			if (!img.isUsedThisFrame) {
				img.lifetime = USED_LIFETIME;
				img.isUsedThisFrame = true;
				return { desc, index };
			}
		}
	}

	Grindstone::GraphicsAPI::Image::CreateInfo createInfo{
		.debugName = "TRACKED Image",
		.width = desc.size.x,
		.height = desc.size.y,
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
		PooledImage{
			.data = TransientImageData {
				.image = image,
				.currentLayout = GraphicsAPI::ImageLayout::Undefined,
				.currentAccessFlags = GraphicsAPI::AccessFlags::None
			},
			.lifetime = USED_LIFETIME,
			.isUsedThisFrame = true,
		}
	);

	return { desc, index };
}

Grindstone::Renderer::TransientBufferKey Grindstone::Renderer::TransientResourceManager::AcquireBuffer(
	const Grindstone::Renderer::BufferDescription& inDesc
) {
	TransientBufferDescription desc = ToTransient(inDesc);

	auto it = buffers.find(desc);
	if (it != buffers.end()) {
		auto& arr = it->second;
		for (size_t index = 0; index < arr.size(); ++index) {
			auto& b = it->second[index];
			if (!b.isUsedThisFrame) {
				b.lifetime = USED_LIFETIME;
				b.isUsedThisFrame = true;
				return { desc, index };
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
		PooledBuffer{
			.data = TransientBufferData {
				.buffer = buffer,
				.currentAccessFlags = GraphicsAPI::AccessFlags::None
			},
			.lifetime = USED_LIFETIME,
			.isUsedThisFrame = true,
		}
	);

	return { desc, index };
}

Grindstone::Renderer::TransientImageData& Grindstone::Renderer::TransientResourceManager::GetTrackedImage(Grindstone::Renderer::TransientImageKey key) {
	TransientImageDescription& desc = key.poolkey;
	auto it = images.find(desc);
	GS_ASSERT(it != images.end());

	return it->second[key.poolIndex].data;
}

Grindstone::Renderer::TransientBufferData& Grindstone::Renderer::TransientResourceManager::GetTrackedBuffer(Grindstone::Renderer::TransientBufferKey key) {
	TransientBufferDescription& desc = key.poolkey;
	auto it = buffers.find(desc);
	GS_ASSERT(it != buffers.end());

	return it->second[key.poolIndex].data;
}
