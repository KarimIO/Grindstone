#include <Common/Graphics/Core.hpp>

#include "TransientResourceManager.hpp"

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
