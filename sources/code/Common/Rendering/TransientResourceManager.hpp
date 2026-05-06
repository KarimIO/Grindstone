#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include <Common/Rect.hpp>
#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/Image.hpp>
#include <Common/Graphics/CommandBuffer.hpp>

#include "AttachmentInfo.hpp"
#include "BufferInfo.hpp"
#include "GpuPassType.hpp"

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class DescriptorSet;
	class Buffer;
	class Image;
}

namespace Grindstone::Renderer {
	struct TransientImageDescription {
		Math::Uint2 size;
		uint32_t samples = 1;
		uint32_t mipLevels = 1;
		uint32_t depth = 1;
		uint32_t arrayLayers = 1;
		Grindstone::GraphicsAPI::Format format;

		GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
		GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage;

		bool operator==(const TransientImageDescription& other) const {
			return
				size == other.size &&
				samples == other.samples &&
				mipLevels == other.mipLevels &&
				arrayLayers == other.arrayLayers &&
				depth == other.depth &&
				format == other.format;
		}
	};

	struct TransientBufferDescription {
		size_t size = 0;
		Grindstone::GraphicsAPI::BufferUsage bufferUsage;
		Grindstone::GraphicsAPI::MemoryUsage memoryUsage;

		bool operator==(const TransientBufferDescription& other) const {
			return size == other.size &&
				bufferUsage == other.bufferUsage &&
				memoryUsage == other.memoryUsage;
		}

		bool operator!=(const TransientBufferDescription& other) const {
			return !(*this == other);
		}
	};

	struct TransientImageData {
		GraphicsAPI::Image* image;
		GraphicsAPI::ImageLayout currentLayout;
		GraphicsAPI::AccessFlags currentAccessFlags;
		Grindstone::GraphicsAPI::PipelineStageBit currentPipelineStage;
	};

	struct TransientBufferData {
		GraphicsAPI::Buffer* buffer;
		GraphicsAPI::AccessFlags currentAccessFlags;
	};

	union TransientResourceUnion {
		TransientImageData image;
		TransientBufferData buffer;
	};
}

namespace std {
	template<>
	struct hash<Grindstone::Renderer::TransientImageDescription> {
		std::size_t operator()(const Grindstone::Renderer::TransientImageDescription& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(desc.size.x) |
				static_cast<size_t>(desc.size.y) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.samples) |
				static_cast<size_t>(desc.mipLevels) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.depth) |
				static_cast<size_t>(desc.arrayLayers) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.format) |
				static_cast<size_t>(desc.imageDimensions) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(desc.memoryUsage) |
				static_cast<size_t>(desc.imageUsage.GetValueUnderlying()) << 32
				);

			return result;
		}
	};

	template<>
	struct hash<Grindstone::Renderer::TransientBufferDescription> {
		std::size_t operator()(const Grindstone::Renderer::TransientBufferDescription& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(desc.bufferUsage) |
				static_cast<size_t>(desc.memoryUsage) << 32
				);

			result ^= std::hash<size_t>{}(static_cast<size_t>(desc.size));
			return result;
		}
	};
}

namespace Grindstone::Renderer {
	struct TransientImageKey {
		TransientImageDescription poolkey;
		size_t poolIndex;
	};

	struct TransientBufferKey {
		TransientBufferDescription poolkey;
		size_t poolIndex;
	};

	class TransientResourceManager {
	public:
		void BeginFrame();

		TransientImageKey AcquireImage(Math::Uint2 viewportResolution, Math::Uint2 swapchainResolution, const ImageDescription& desc);
		TransientBufferKey AcquireBuffer(const BufferDescription& desc);

		Grindstone::Renderer::TransientImageData& GetTrackedImage(TransientImageKey key);
		Grindstone::Renderer::TransientBufferData& GetTrackedBuffer(TransientBufferKey key);

	protected:

		struct PooledImage {
			TransientImageData data;
			int8_t lifetime;
			bool isUsedThisFrame = false;
		};

		struct PooledBuffer {
			TransientBufferData data;
			int8_t lifetime;
			bool isUsedThisFrame = false;
		};

		std::unordered_map<TransientImageDescription, std::vector<PooledImage>> images;
		std::unordered_map<TransientBufferDescription, std::vector<PooledBuffer>> buffers;

	};
}
