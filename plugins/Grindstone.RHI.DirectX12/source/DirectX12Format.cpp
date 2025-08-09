#include "DirectX12Format.hpp"
#include <stdio.h>
#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VkFilter TranslateFilterToDirectX12(TextureFilter f) {
			switch (f) {
			case TextureFilter::Nearest:
				return VK_FILTER_NEAREST;
			case TextureFilter::Linear:
			case TextureFilter::NearestMipMapNearest:
			case TextureFilter::NearestMipMapLinear:
			case TextureFilter::LinearMipMapNearest:
			case TextureFilter::LinearMipMapLinear:
				return VK_FILTER_LINEAR;
			}
		}

		VkSamplerAddressMode TranslateWrapToDirectX12(TextureWrapMode m) {
			switch (m) {
			case TextureWrapMode::Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case TextureWrapMode::ClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case TextureWrapMode::ClampToBorder:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case TextureWrapMode::MirroredRepeat:
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case TextureWrapMode::MirroredClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			}
		}

		VkFormat TranslateVertexFormatsToDirectX12(VertexFormat format) {
			switch (format) {
			case VertexFormat::R32_G32:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexFormat::R32_G32_B32:
				return VK_FORMAT_R32G32_SFLOAT;
			}
		}

		ColorFormat TranslateColorFormatFromDirectX12(VkFormat format) {
			switch (format) {
			case VK_FORMAT_R8_UNORM:
				return ColorFormat::R8;
			case VK_FORMAT_R8G8_UNORM:
				return ColorFormat::R8G8;
			case VK_FORMAT_B8G8R8_UINT:
				return ColorFormat::R8G8B8;
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_UNORM:
				return ColorFormat::R8G8B8A8;
			default:
				printf("Invalid color format!");
				assert(false);
				break;
			}
			return ColorFormat::R8G8B8A8;
		}
		
		DepthFormat TranslateDepthFormatFromDirectX12(VkFormat format) {
			switch (format) {
			case VK_FORMAT_D16_UNORM:
				return DepthFormat::D16;
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				return DepthFormat::D24;
			case VK_FORMAT_D32_SFLOAT:
				return DepthFormat::D32;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				return DepthFormat::D32_STENCIL_8;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return DepthFormat::D32_STENCIL_8;
			default:
				printf("Invalid depth format!");
				assert(false);
				break;
				//case DepthFormat::D16_STENCIL_8:
				//		return VK_FORMAT_D16_UNORM_S8_UINT;
				//case FORMAT_STENCIL_8:
				//	return VK_FORMAT_S8_UINT;
			}
			return DepthFormat::D24;
		}

		VkFormat TranslateColorFormatToDirectX12(ColorFormat in, uint8_t &channels) {
			switch (in) {
			case ColorFormat::R8:
				channels = 1;
				return VK_FORMAT_R8_UNORM;
			case ColorFormat::R8G8:
				channels = 2;
				return VK_FORMAT_R8G8_UNORM;
			case ColorFormat::R8G8B8:
				channels = 3;
				return VK_FORMAT_B8G8R8_UINT;
			case ColorFormat::R8G8B8A8:
				channels = 4;
				return VK_FORMAT_B8G8R8A8_UNORM;
			default:
				printf("Invalid color format!");
				assert(false);
				break;
			}
			return VK_FORMAT_B8G8R8A8_UNORM;
		}

		VkFormat TranslateDepthFormatToDirectX12(DepthFormat in) {
			switch (in) {
			case DepthFormat::D16:
				return VK_FORMAT_D16_UNORM;
			case DepthFormat::D24:
				return VK_FORMAT_X8_D24_UNORM_PACK32;
			case DepthFormat::D32:
				return VK_FORMAT_D32_SFLOAT;
			case DepthFormat::D24_STENCIL_8:
				return VK_FORMAT_D24_UNORM_S8_UINT;
			case DepthFormat::D32_STENCIL_8:
				return VK_FORMAT_D32_SFLOAT_S8_UINT;
			default:
				printf("Invalid depth format!");
				assert(false);
				break;
			//case DepthFormat::D16_STENCIL_8:
			//		return VK_FORMAT_D16_UNORM_S8_UINT;
			//case FORMAT_STENCIL_8:
			//	return VK_FORMAT_S8_UINT;
			}
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		}
	}
}
