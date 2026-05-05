#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <stdint.h>

#include <Common/EnumTraits.hpp>
#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Image.hpp>

#include "RenderGraphResourceRef.hpp"

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

		bool operator==(const MetaSize& other) const {
			return value == other.value;
		}

		explicit operator uint32_t() const {
			return value;
		}

		static const MetaSize Pixels(uint32_t x) { return { .value = x }; };
		static const MetaSize Viewport() { return { static_cast<uint32_t>(ImageSizeInfo::ViewportRelative) }; };
		static const MetaSize Swapchain() { return { static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative) }; };
		static MetaSize MultiplyViewport(uint32_t x) { return { static_cast<uint32_t>(ImageSizeInfo::ViewportRelative) | static_cast<uint32_t>(ImageSizeInfo::Multiply) | x }; }
		static MetaSize MultiplySwapchain(uint32_t x) { return { static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative) | static_cast<uint32_t>(ImageSizeInfo::Multiply) | x }; }

		static MetaSize DivideViewport(uint32_t x) { return { static_cast<uint32_t>(ImageSizeInfo::ViewportRelative) | x }; }
		static MetaSize DivideSwapchain(uint32_t x) { return { static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative) | x }; }

		inline bool IsSwapchainRelative() const {
			return value & static_cast<uint32_t>(ImageSizeInfo::SwapchainRelative);
		}

		inline bool IsViewportRelative() const {
			return value & static_cast<uint32_t>(ImageSizeInfo::ViewportRelative);
		}

		inline bool IsMultiply() const {
			return value & static_cast<uint32_t>(ImageSizeInfo::Multiply);
		}

		inline bool IsDivide() const {
			return (value & static_cast<uint32_t>(ImageSizeInfo::Multiply)) == 0;
		}

		uint32_t Resolve(uint32_t viewportResolution, uint32_t swapchainResolution) const {
			uint32_t size = value & static_cast<uint32_t>(ImageSizeInfo::SizeBits);
			bool isAbsoluteSize = (size == 0);

			if (IsViewportRelative()) {
				if (IsMultiply()) {
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
			else if (IsSwapchainRelative()) {
				if (IsMultiply()) {
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

	struct MetaSize2D {
		MetaSize x;
		MetaSize y;

		static const MetaSize2D Zero() { return { 0, 0 }; };

		static const MetaSize2D Pixels(uint32_t x, uint32_t y) { return { MetaSize::Pixels(x), MetaSize::Pixels(y) }; };
		static const MetaSize2D Viewport() { return { MetaSize::Viewport(), MetaSize::Viewport() }; };
		static const MetaSize2D Swapchain() { return { MetaSize::Swapchain(), MetaSize::Swapchain() }; };

		static MetaSize2D MultiplyViewport(uint32_t x) { return { MetaSize::MultiplyViewport(x), MetaSize::MultiplyViewport(x) }; }
		static MetaSize2D MultiplySwapchain(uint32_t x) { return { MetaSize::MultiplySwapchain(x), MetaSize::MultiplySwapchain(x) }; }

		static MetaSize2D DivideViewport(uint32_t x) { return { MetaSize::DivideViewport(x), MetaSize::DivideViewport(x) }; }
		static MetaSize2D DivideSwapchain(uint32_t x) { return { MetaSize::DivideSwapchain(x), MetaSize::DivideSwapchain(x) }; }

		bool operator==(const MetaSize2D& other) const {
			return x == other.x && y == other.y;
		}

		Grindstone::Math::Uint2 Resolve(Grindstone::Math::Uint2 viewportResolution, Grindstone::Math::Uint2 swapchainResolution) const {
			return { x.Resolve(viewportResolution.x, swapchainResolution.x), y.Resolve(viewportResolution.y, swapchainResolution.y) };
		}

		Grindstone::Math::Uint2 Resolve(Grindstone::Math::Uint2 swapchainResolution) const {
			GS_ASSERT(!x.IsViewportRelative());
			GS_ASSERT(!y.IsViewportRelative());
			return { x.Resolve(0, swapchainResolution.x), y.Resolve(0, swapchainResolution.y) };
		}
	};

	struct MetaRect {
		MetaSize2D offset;
		MetaSize2D extent;

		static const MetaRect Pixels(uint32_t width, uint32_t height) {
			return { MetaSize2D::Zero(), MetaSize2D::Pixels(width, height) };
		}
		static const MetaRect Pixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
			return { MetaSize2D::Pixels(x, y), MetaSize2D::Pixels(width, height) };
		}
		static const MetaRect Viewport() { return { MetaSize2D::Zero(), MetaSize2D::Viewport() }; }
		static const MetaRect Swapchain() { return  { MetaSize2D::Zero(), MetaSize2D::Swapchain() }; }

		bool operator==(const MetaRect& other) const {
			return offset == other.offset && extent == other.extent;
		}

		Grindstone::Math::IntRect2D Resolve(Grindstone::Math::Uint2 viewportResolution, Grindstone::Math::Uint2 swapchainResolution) const {
			return { offset.Resolve(viewportResolution, swapchainResolution), extent.Resolve(viewportResolution, swapchainResolution) };
		}

		Grindstone::Math::IntRect2D Resolve(Grindstone::Math::Uint2 swapchainResolution) const {
			return { offset.Resolve(swapchainResolution), extent.Resolve(swapchainResolution) };
		}
	};

	enum class RenderGraphImageUsage : uint16_t {
		None = 0,
		Read = 1 << 0,
		Write = 1 << 1,
		Sampled = 1 << 2,
		Storage = 1 << 3,
		ColorAttachment = 1 << 4,
		DepthAttachment = 1 << 5,
		StencilAttachment = 1 << 6,
		SubpassInputAttachment = 1 << 7,
		Transfer = 1 << 8,

		SampledRead = static_cast<uint16_t>(RenderGraphImageUsage::Sampled) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		StorageRead = static_cast<uint16_t>(RenderGraphImageUsage::Storage) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		StorageWrite = static_cast<uint16_t>(RenderGraphImageUsage::Storage) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		StorageReadWrite = static_cast<uint16_t>(RenderGraphImageUsage::StorageRead) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		ColorAttachmentRead = static_cast<uint16_t>(RenderGraphImageUsage::ColorAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		ColorAttachmentWrite = static_cast<uint16_t>(RenderGraphImageUsage::ColorAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		ColorAttachmentReadWrite = static_cast<uint16_t>(RenderGraphImageUsage::ColorAttachmentRead) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		DepthAttachmentRead = static_cast<uint16_t>(RenderGraphImageUsage::DepthAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		DepthAttachmentWrite = static_cast<uint16_t>(RenderGraphImageUsage::DepthAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		DepthAttachmentReadWrite = static_cast<uint16_t>(RenderGraphImageUsage::DepthAttachmentRead) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		StencilAttachmentRead = static_cast<uint16_t>(RenderGraphImageUsage::StencilAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		StencilAttachmentWrite = static_cast<uint16_t>(RenderGraphImageUsage::StencilAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		StencilAttachmentReadWrite = static_cast<uint16_t>(RenderGraphImageUsage::StencilAttachmentRead) | static_cast<uint16_t>(RenderGraphImageUsage::Write),
		DepthStencilAttachmentRead = static_cast<uint16_t>(RenderGraphImageUsage::DepthAttachmentRead) | static_cast<uint16_t>(RenderGraphImageUsage::StencilAttachment),
		DepthStencilAttachmentWrite = static_cast<uint16_t>(RenderGraphImageUsage::DepthAttachmentWrite) | static_cast<uint16_t>(RenderGraphImageUsage::StencilAttachment),
		DepthStencilAttachmentReadWrite = static_cast<uint16_t>(RenderGraphImageUsage::DepthAttachmentReadWrite) | static_cast<uint16_t>(RenderGraphImageUsage::StencilAttachment),
		SubpassInputAttachmentRead = static_cast<uint16_t>(RenderGraphImageUsage::SubpassInputAttachment) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		TransferSrc = static_cast<uint16_t>(RenderGraphImageUsage::Transfer) | static_cast<uint16_t>(RenderGraphImageUsage::Read),
		TransferDst = static_cast<uint16_t>(RenderGraphImageUsage::Transfer) | static_cast<uint16_t>(RenderGraphImageUsage::Write)
	};
}

GS_ENUM_FLAGS_FUNCS(Grindstone::Renderer::RenderGraphImageUsage)

namespace Grindstone::Renderer {
	struct PassImageDesc {
		RenderGraphBuilderResourceRef	ref;
		RenderGraphImageUsage			usage;

		struct AttachmentMeta {
			Grindstone::GraphicsAPI::LoadOp			loadOp;
			Grindstone::GraphicsAPI::ClearUnion		clearValue;
		};

		AttachmentMeta attachment;

		bool IsRead() const {
			return Any(usage & RenderGraphImageUsage::Read);
		}

		bool IsWrite() const {
			return Any(usage & RenderGraphImageUsage::Write);
		}

		bool IsTransfer() const {
			return Any(usage & RenderGraphImageUsage::Transfer);
		}

		bool IsShaderInput() const {
			return Any(usage & (RenderGraphImageUsage::Sampled | RenderGraphImageUsage::Storage));
		}

		bool IsAttachment() const {
			return Any(usage & (RenderGraphImageUsage::ColorAttachment | RenderGraphImageUsage::DepthAttachment | RenderGraphImageUsage::StencilAttachment));
		}
	};

	struct ImageDescription {
		Grindstone::String name;
		MetaSize2D size = MetaSize2D::Viewport();
		uint32_t samples = 1;
		uint32_t mipLevels = 1;
		uint32_t depth = 1;
		uint32_t arrayLayers = 1;
		Grindstone::GraphicsAPI::Format format;

		GraphicsAPI::ImageDimension imageDimensions = GraphicsAPI::ImageDimension::Dimension2D;
		GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
		Grindstone::Containers::BitsetFlags<GraphicsAPI::ImageUsageFlags> imageUsage;
		std::function<Grindstone::GraphicsAPI::Image*()> externalGetterCallback;

		bool operator==(const ImageDescription& other) const {
			return 
				size == other.size &&
				samples == other.samples &&
				mipLevels == other.mipLevels &&
				arrayLayers == other.arrayLayers &&
				depth == other.depth &&
				format == other.format;
		}
	};

	struct AttachmentInfo {
		Grindstone::Renderer::RenderGraphBuilderResourceRef imageResourceIndex;
		Grindstone::GraphicsAPI::LoadOp loadOp;
		Grindstone::GraphicsAPI::ClearUnion clearValue;
	};
}

namespace std {
	template<>
	struct hash<Grindstone::Renderer::MetaSize> {
		std::size_t operator()(const Grindstone::Renderer::MetaSize& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(desc.value)
			);
		}
	};

	template<>
	struct hash<Grindstone::Renderer::MetaSize2D> {
		std::size_t operator()(const Grindstone::Renderer::MetaSize2D& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(static_cast<uint32_t>(desc.x)) |
				static_cast<size_t>(static_cast<uint32_t>(desc.y)) << 32
			);
		}
	};

	template<>
	struct hash<Grindstone::Renderer::MetaRect> {
		std::size_t operator()(const Grindstone::Renderer::MetaRect& desc) const noexcept {
			size_t result =
				std::hash<Grindstone::Renderer::MetaSize2D>{}(desc.offset) ^
				std::hash<Grindstone::Renderer::MetaSize2D>{}(desc.extent);
		}
	};

	template<>
	struct hash<Grindstone::Renderer::ImageDescription> {
		std::size_t operator()(const Grindstone::Renderer::ImageDescription& desc) const noexcept {
			size_t result = std::hash<Grindstone::Renderer::MetaSize2D>{}(desc.size);

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
}

