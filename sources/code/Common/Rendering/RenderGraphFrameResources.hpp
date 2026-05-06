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
			Grindstone::GraphicsAPI::AccessFlags access,
			Grindstone::GraphicsAPI::PipelineStageBit pipelineStage
		) {
			auto externalIt = externalImages.find(id);
			if (externalIt != externalImages.end()) {
				TransientImageData& externalResource = externalIt->second;
				externalResource.currentLayout = layout;
				externalResource.currentAccessFlags = access;
				externalResource.currentPipelineStage = pipelineStage;
				return;
			}

			auto& transientResource = *images.at(id);
			transientResource.currentLayout = layout;
			transientResource.currentAccessFlags = access;
			transientResource.currentPipelineStage = pipelineStage;
		}

		std::tuple<Grindstone::GraphicsAPI::ImageLayout, Grindstone::GraphicsAPI::AccessFlags, Grindstone::GraphicsAPI::PipelineStageBit> GetLayout(
			Grindstone::Renderer::ResourceId id
		) {
			auto externalIt = externalImages.find(id);
			if (externalIt != externalImages.end()) {
				TransientImageData& externalResource = externalIt->second;
				return { externalResource.currentLayout, externalResource.currentAccessFlags, externalResource.currentPipelineStage };
			}

			TransientImageData& transientResource = *images.at(id);
			return { transientResource.currentLayout, transientResource.currentAccessFlags, transientResource.currentPipelineStage };
		}
	};
}
