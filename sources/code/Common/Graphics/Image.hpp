#pragma once

#include <stdint.h>

#include <Common/Containers/Bitset.hpp>
#include <Common/Buffer.hpp>

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
		static const uint64_t MAPPED_MEMORY_ENTIRE_BUFFER = UINT64_MAX;

		struct CreateInfo {
			const char* debugName;
			uint32_t width = 1;
			uint32_t height = 1;
			uint32_t depth = 1;
			uint32_t mipLevels = 1;
			uint32_t arrayLayers = 1;

			GraphicsAPI::Format format = GraphicsAPI::Format::Invalid;
			GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
			GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
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

		Image() = default;
		Image(const Image&) = default;
		Image(Image&&) noexcept = default;
		Image& operator=(const Image&) = default;
		Image& operator=(Image&&) noexcept = default;

		Image(
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			uint32_t mipLevels,
			uint32_t arrayLayers,
			uint64_t maxImageSize,
			GraphicsAPI::ImageDimension imageDimension,
			GraphicsAPI::Format format,
			Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage,
			GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly
		) : width(width),
			height(height),
			depth(depth),
			mipLevels(mipLevels),
			arrayLayers(arrayLayers),
			maxImageSize(maxImageSize),
			imageDimension(imageDimension),
			format(format),
			imageUsage(imageUsage),
			memoryUsage(memoryUsage) {}

		bool IsCubemap() const { return imageUsage.Test(GraphicsAPI::ImageUsageFlags::Cubemap); }
		uint32_t GetWidth() const { return width; }
		uint32_t GetHeight() const { return height; }
		uint32_t GetDepth() const { return depth; }
		uint32_t GetMipLevels() const { return mipLevels; }
		uint32_t GetArrayLayers() const { return arrayLayers; }
		uint64_t GetMaxImageSize() const { return maxImageSize; }
		GraphicsAPI::ImageDimension GetImageDimension() const { return imageDimension; }
		GraphicsAPI::Format GetFormat() const { return format; }
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> GetImageUsage() const { return imageUsage; }
		GraphicsAPI::MemoryUsage GetMemoryUsage() const { return memoryUsage; }

		// Discards the previous image and creates a new image of a certain size, useful for RenderTargets.
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		// Upload data to the entire image based on the data from the CreateInfo.
		virtual void UploadData(const char* data, uint64_t dataSize) = 0;

		// Obtain a CPU poitner to GPU memory.
		virtual void* MapMemory(uint64_t dataSize = MAPPED_MEMORY_ENTIRE_BUFFER, uint64_t dataOffset = 0) = 0;

		// Stop reading memory.
		virtual void UnmapMemory() = 0;

		// Upload data to specific regions for more control, used especially for creating Texture Atlases or other
		// more specific graphical techniques. A buffer is passed in, and regions map specific parts of memory to
		// coordinates, dimensions, array layers, and mip levels of the image.
		virtual void UploadDataRegions(void* buffer, size_t bufferSize, ImageRegion* regions, uint32_t regionCount) = 0;

		// Readback memory using a staging buffer, and then copy that to a Grindstone::Buffer.
		virtual Grindstone::Buffer ReadbackMemory() = 0;

	protected:

		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t mipLevels;
		uint32_t arrayLayers;
		uint64_t maxImageSize;
		GraphicsAPI::ImageDimension imageDimension;
		GraphicsAPI::Format format;
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage;
		GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;

	};
}
