#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Image.hpp>

namespace Grindstone::Renderer {
	struct MetaSize {
		uint32_t value;

		enum class ImageSizeInfo : uint32_t {
			// If this bit is disabled, the base of the image size is divided, if it is enabled, it is multiplied, by the size it is relative to.
			Multiply = 1u << 29u,

			// If neither this bit, nor SwapchainRelative is enabled, the size is absolute
			ViewportRelative = 1u << 30u,

			// If neither this bit, nor ViewportRelative is enabled, the size is absolute
			SwapchainRelative = 1u << 31u,

			MetaBits = Multiply | ViewportRelative | SwapchainRelative,
			SizeBits = ~static_cast<uint32_t>(MetaBits)
		};

		static const MetaSize Viewport() { MetaSize{ static_cast<uint32_t>(ImageSizeInfo::ViewportRelative) }; };
		static const MetaSize Swapchain() { MetaSize{ static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative) }; };
		static MetaSize MultiplyViewport(uint32_t x) { MetaSize{ static_cast<uint32_t>(ImageSizeInfo::ViewportRelative) | static_cast<uint32_t>(ImageSizeInfo::Multiply) | x }; }
		static MetaSize MultiplySwapchain(uint32_t x) { MetaSize{ static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative) | static_cast<uint32_t>(ImageSizeInfo::Multiply) | x }; }

		static MetaSize DivideViewport(uint32_t x) { MetaSize{ static_cast<uint32_t>(ImageSizeInfo::ViewportRelative) | x }; }
		static MetaSize DivideSwapchain(uint32_t x) { MetaSize{ static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative) | x }; }

		constexpr uint32_t Resolve(uint32_t viewportResolution, uint32_t swapchainResolution, uint32_t sizeInformation) {
			uint32_t size = sizeInformation & static_cast<uint32_t>(ImageSizeInfo::SizeBits);
			bool isAbsoluteSize = (size == 0);

			if (sizeInformation & static_cast<uint32_t>(ImageSizeInfo::ViewportRelative)) {
				if (sizeInformation & static_cast<uint32_t>(ImageSizeInfo::Multiply)) {
					return isAbsoluteSize
						? viewportResolution
						: viewportResolution * size;
				}
				else {
					return isAbsoluteSize
						? viewportResolution
						: viewportResolution / size;
				}
			}
			else if (sizeInformation & static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative)) {
				if (sizeInformation & static_cast<uint32_t>(ImageSizeInfo::Multiply)) {
					return isAbsoluteSize
						? swapchainResolution
						: swapchainResolution * size;
				}
				else {
					return isAbsoluteSize
						? swapchainResolution
						: swapchainResolution / size;
				}
			}
			else {
				return size;
			}
		}
	};

	struct ImageDescription {
		MetaSize width;
		MetaSize height;
		uint32_t samples = 1;
		uint32_t mipLevels = 1;
		uint32_t depth = 1;
		uint32_t arrayLayers = 1;
		Grindstone::GraphicsAPI::Format format;

		GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
		GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage;

		bool operator==(const ImageDescription& other) const {
			return 
				width == other.width &&
				height == other.height &&
				samples == other.samples &&
				mipLevels == other.mipLevels &&
				arrayLayers == other.arrayLayers &&
				depth == other.depth &&
				format == other.format;
		}
	};
}
