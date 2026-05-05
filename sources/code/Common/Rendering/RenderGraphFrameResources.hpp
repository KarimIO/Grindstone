#pragma once

#include <unordered_map>

#include "TransientResourceManager.hpp"

namespace Grindstone::Renderer {
	struct RenderGraphFrameResources {
		std::unordered_map<Grindstone::Renderer::ResourceId, Grindstone::Renderer::TransientImageData>		externalImages;
		std::unordered_map<Grindstone::Renderer::ResourceId, Grindstone::Renderer::TransientBufferData>		externalBuffers;
		std::unordered_map<Grindstone::Renderer::ResourceId, Grindstone::Renderer::TransientImageKey>		imageKeys;
		std::unordered_map<Grindstone::Renderer::ResourceId, Grindstone::Renderer::TransientBufferKey>		bufferKeys;
		std::unordered_map<Grindstone::Renderer::ResourceId, Grindstone::Renderer::TransientImageData*>		images;
		std::unordered_map<Grindstone::Renderer::ResourceId, Grindstone::Renderer::TransientBufferData*>	buffers;

		void RealizeKeys(Grindstone::Renderer::TransientResourceManager* resourceManager) {
			for (auto& [id, key] : imageKeys) {
				if (key.poolIndex == SIZE_MAX) {
					images[id] = &externalImages[id];
				}
				else {
					images[id] = &resourceManager->GetTrackedImage(key);
				}
			}

			for (auto& [id, key] : bufferKeys) {
				buffers[id] = &resourceManager->GetTrackedBuffer(key);
			}
		}

		Grindstone::GraphicsAPI::Image* GetImage(Grindstone::Renderer::ResourceId id) {
			auto externalIt = externalImages.find(id);
			if (externalIt != externalImages.end()) {
				return externalIt->second.image;
			}

			auto it = images.find(id);
			GS_ASSERT(it != images.end());
			return it->second->image;
		}

		Grindstone::GraphicsAPI::Buffer* GetBuffer(Grindstone::Renderer::ResourceId id) {
			auto it = buffers.find(id);
			GS_ASSERT(it != buffers.end());
			return it->second->buffer;
		}

		// Layout tracking - updated as passes execute and emit barriers
		void SetLayout(
			Grindstone::Renderer::ResourceId id,
			Grindstone::GraphicsAPI::ImageLayout layout,
			Grindstone::GraphicsAPI::AccessFlags access
		) {
			auto externalIt = externalImages.find(id);
			if (externalIt != externalImages.end()) {
				TransientImageData& externalResource = externalIt->second;
				externalResource.currentLayout = layout;
				externalResource.currentAccessFlags = access;
				return;
			}

			images.at(id)->currentLayout = layout;
			images.at(id)->currentAccessFlags = access;
		}

		Grindstone::GraphicsAPI::ImageLayout GetLayout(Grindstone::Renderer::ResourceId id) {
			return images.at(id)->currentLayout;
		}
	};
}
