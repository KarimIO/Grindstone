#pragma once

#include <stdint.h>

#include <Common/Containers/Bitset.hpp>

#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	enum class ImageUsageFlags : uint8_t {
		Sampled = 1,
		RenderTarget = 1 << 1,
		DepthStencil = 1 << 2,
		Storage = 1 << 3,
		GenerateMipmaps = 1 << 4,
		Cubemap = 1 << 5,
		TransferSrc = 1 << 6,
		TransferDst = 1 << 7,
		Count = 8
	};
}

template <>
struct EnumFlagsTraits<Grindstone::GraphicsAPI::ImageUsageFlags> {
	static constexpr const char* names[] = {
		"Sampled",
		"RenderTarget",
		"DepthStencil",
		"Storage",
		"GenerateMipmaps",
		"Cubemap",
		"TransferSrc",
		"TransferDst"
	};
	static constexpr size_t size = 8;
};

inline Grindstone::GraphicsAPI::ImageUsageFlags operator|(Grindstone::GraphicsAPI::ImageUsageFlags a, const Grindstone::GraphicsAPI::ImageUsageFlags b) {
	using Underlying = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ImageUsageFlags>(static_cast<Underlying>(a) | static_cast<Underlying>(b));
}

namespace Grindstone::GraphicsAPI {
	/*! Images represent pixel data. They can include a multitude of formats
		representing red, green, blue, and alpha channels. They represent 1d,
		2d, or 3d buffers, cubemaps, etc. These also include sampler information.
	*/
	class Image {
	public:
		struct CreateInfo {
			const char* debugName;
			uint32_t width = 1;
			uint32_t height = 1;
			uint32_t depth = 1;
			uint32_t mipLevels = 1;
			uint32_t arrayLayers = 1;

			GraphicsAPI::Format format = GraphicsAPI::Format::Invalid;
			GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
			Grindstone::Containers::BitsetFlags<ImageUsageFlags> imageUsage;

			const char* initialData = nullptr;
			uint64_t initialDataSize = 0;
		};

		// ImageRegions are used to map regions to a buffer thatis passed in, and mapping specific parts of memory to
		// coordinates, dimensions, array layers, and mip levels of the image. It is used with UploadDataRegions.
		struct ImageRegion {
			uint64_t bufferOffset = 0;
			uint32_t bufferRowLength = 0;
			uint32_t bufferImageHeight = 0;
			int32_t x = 0;
			int32_t y = 0;
			int32_t z = 0;
			uint32_t width = 1;
			uint32_t height = 1;
			uint32_t depth = 1;
			uint32_t mipLevel = 0;
			uint32_t baseArrayLayer = 0;
			uint32_t arrayLayerCount = 1;
		};

		// Discards the previous image and creates a new image of a certain size, useful for RenderTargets.
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		// Upload data to the entire image based on the data from the CreateInfo.
		virtual void UploadData(const char* data, uint64_t dataSize) = 0;

		// Upload data to specific regions for more control, used especially for creating Texture Atlases or other
		// more specific graphical techniques. A buffer is passed in, and regions map specific parts of memory to
		// coordinates, dimensions, array layers, and mip levels of the image.
		virtual void UploadDataRegions(void* buffer, size_t bufferSize, ImageRegion* regions, uint32_t regionCount) = 0;
	};
}
