#include "VulkanFormat.hpp"
#include <stdio.h>
#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VkFilter TranslateFilterToVulkan(TextureFilter f) {
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

			assert(false && "TranslateFilterToVulkan: Invalid TextureFilter!");
			return VK_FILTER_LINEAR;
		}

		VkSamplerAddressMode TranslateWrapToVulkan(TextureWrapMode m) {
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

			assert(false && "TranslateWrapToVulkan: Invalid TextureWrapMode!");
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}

		VkFormat TranslateVertexFormatsToVulkan(VertexFormat format) {
			switch (format) {
			case VertexFormat::Float3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexFormat::Float2:
				return VK_FORMAT_R32G32_SFLOAT;
			}

			assert(false && "TranslateVertexFormatsToVulkan: Invalid Vertex Format!");
			return VK_FORMAT_R32G32B32_SFLOAT;
		}

		ColorFormat TranslateColorFormatFromVulkan(VkFormat format) {
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
			}

			assert(false && "TranslateColorFormatFromVulkan: Invalid color format!");
			return ColorFormat::R8G8B8A8;
		}
		
		DepthFormat TranslateDepthFormatFromVulkan(VkFormat format) {
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
			//case DepthFormat::D16_STENCIL_8:
			//		return VK_FORMAT_D16_UNORM_S8_UINT;
			//case FORMAT_STENCIL_8:
			//	return VK_FORMAT_S8_UINT;
			}

			assert(false && "TranslateDepthFormatFromVulkan: Invalid depth format!");
			return DepthFormat::D24;
		}

		VkFormat TranslateColorFormatToVulkan(ColorFormat in, uint8_t &channels) {
			switch (in) {
			case ColorFormat::R8:
				channels = 1;
				return VK_FORMAT_R8_UNORM;
			case ColorFormat::R8G8:
				channels = 2;
				return VK_FORMAT_R8G8_UNORM;
			case ColorFormat::R8G8B8:
				channels = 3;
				return VK_FORMAT_B8G8R8_UNORM;
			case ColorFormat::R8G8B8A8:
				channels = 4;
				return VK_FORMAT_B8G8R8A8_UNORM;

			case ColorFormat::R10G10B10A2:
				channels = 4;
				return VK_FORMAT_A2R10G10B10_UNORM_PACK32;

			case ColorFormat::R16:
				channels = 1;
				return VK_FORMAT_R16_SFLOAT;
			case ColorFormat::R16G16:
				channels = 2;
				return VK_FORMAT_R16G16_SFLOAT;
			case ColorFormat::R16G16B16:
				channels = 3;
				return VK_FORMAT_R16G16B16_SFLOAT;
			case ColorFormat::R16G16B16A16:
				channels = 4;
				return VK_FORMAT_R16G16B16A16_SFLOAT;

			case ColorFormat::R32G32B32:
				channels = 3;
				return VK_FORMAT_R32G32B32_SFLOAT;
			case ColorFormat::R32G32B32A32:
				channels = 4;
				return VK_FORMAT_R32G32B32A32_SFLOAT;

			case ColorFormat::RGB_DXT1:
				channels = 4;
				return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
			case ColorFormat::RGBA_DXT1:
				channels = 4;
				return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case ColorFormat::RGBA_DXT3:
				channels = 4;
				return VK_FORMAT_BC2_UNORM_BLOCK;
			case ColorFormat::RGBA_DXT5:
				channels = 4;
				return VK_FORMAT_BC3_UNORM_BLOCK;
			case ColorFormat::SRGB_DXT1:
				channels = 3;
				return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
			case ColorFormat::SRGB_ALPHA_DXT1:
				channels = 4;
				return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			case ColorFormat::SRGB_ALPHA_DXT3:
				channels = 4;
				return VK_FORMAT_BC2_SRGB_BLOCK;
			case ColorFormat::SRGB_ALPHA_DXT5:
				channels = 4;
				return VK_FORMAT_BC3_SRGB_BLOCK;
			case ColorFormat::BC4:
				channels = 1;
				return VK_FORMAT_BC4_UNORM_BLOCK;
			default:
				printf("Invalid color format!");
				assert(false);
				break;
			}

			assert(false && "TranslateColorFormatToVulkan: Invalid color format!");
			return VK_FORMAT_B8G8R8A8_UNORM;
		}

		VkFormat TranslateDepthFormatToVulkan(DepthFormat in, bool& hasStencil) {
			switch (in) {
			case DepthFormat::D16:
				hasStencil = false;
				return VK_FORMAT_D16_UNORM;
			case DepthFormat::D24:
				hasStencil = false;
				return VK_FORMAT_X8_D24_UNORM_PACK32;
			case DepthFormat::D32:
				hasStencil = false;
				return VK_FORMAT_D32_SFLOAT;
			case DepthFormat::D24_STENCIL_8:
				hasStencil = true;
				return VK_FORMAT_D24_UNORM_S8_UINT;
			case DepthFormat::D32_STENCIL_8:
				hasStencil = true;
				return VK_FORMAT_D32_SFLOAT_S8_UINT;
			//case DepthFormat::D16_STENCIL_8:
			//		return VK_FORMAT_D16_UNORM_S8_UINT;
			//case FORMAT_STENCIL_8:
			//	return VK_FORMAT_S8_UINT;
			}

			assert(false && "TranslateDepthFormatToVulkan: Invalid DepthFormat!");
			hasStencil = true;
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		}

		VkCullModeFlags TranslateCullModeToVulkan(CullMode cullMode) {
			switch (cullMode) {
			case CullMode::None:
				return VK_CULL_MODE_NONE;
			case CullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case CullMode::Back:
				return VK_CULL_MODE_BACK_BIT;
			case CullMode::Both:
				return VK_CULL_MODE_FRONT_AND_BACK;
			}

			assert(false && "TranslateCullModeToVulkan: Invalid CullMode format!");
			return VK_CULL_MODE_NONE;
		}
	}
}
