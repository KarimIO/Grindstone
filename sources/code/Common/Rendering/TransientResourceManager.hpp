#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include <Common/Rect.hpp>
#include "GpuQueue.hpp"

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/Image.hpp>
#include <Common/Graphics/CommandBuffer.hpp>

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class DescriptorSet;
	class Buffer;
	class Image;
}

namespace Grindstone::Renderer {
	struct TransientImageData {
		GraphicsAPI::Image* image;
		GraphicsAPI::ImageLayout currentLayout;
		GraphicsAPI::AccessFlags currentAccessFlags;
	};

	struct TransientBufferData {
		GraphicsAPI::Buffer* buffer;
		GraphicsAPI::AccessFlags currentAccessFlags;
	};

	union TransientResourceUnion {
		TransientImageData image;
		TransientBufferData buffer;
	};

	class TransientResourceManager {
	public:

		void StartFrame();

		std::tuple<TransientImageData, size_t> AddTrackedImage(uint32_t viewportWidth, uint32_t viewportHeight, ImageDescription desc);
		std::tuple<TransientBufferData, size_t> AddTrackedBuffer(BufferDescription desc);

		Grindstone::Renderer::TransientImageData& GetTrackedImage(uint32_t viewportWidth, uint32_t viewportHeight, ImageDescription inDesc, size_t index);
		Grindstone::Renderer::TransientBufferData& GetTrackedBuffer(BufferDescription desc, size_t index);

	protected:

		struct TransientImage {
			bool isUsedThisFrame = false;
			int8_t lifetime;
			TransientImageData data;
		};

		struct TransientBuffer {
			bool isUsedThisFrame = false;
			int8_t lifetime;
			TransientBufferData data;
		};
		std::unordered_map<TransientImageDescription, std::vector<TransientImage>> images;
		std::unordered_map<BufferDescription, std::vector<TransientBuffer>> buffers;

	};
}
