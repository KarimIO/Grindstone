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

			Format format;
			Grindstone::Containers::BitsetFlags<ImageUsageFlags> imageUsage;

			const char* initialData = nullptr;
			uint64_t initialDataSize = 0;
		};

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void UploadData(const char* data, uint64_t dataSize) = 0;
	};
}
