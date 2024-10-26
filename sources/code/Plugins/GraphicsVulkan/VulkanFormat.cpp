#include "VulkanFormat.hpp"
#include <stdio.h>
#include <assert.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	VkFilter TranslateFilterToVulkan(TextureFilter f) {
		switch (f) {
		case TextureFilter::Nearest:
			return VK_FILTER_NEAREST;
		case TextureFilter::Linear:
			return VK_FILTER_LINEAR;
		}

		assert(false && "TranslateFilterToVulkan: Invalid TextureFilter!");
		return VK_FILTER_LINEAR;
	}

	VkSamplerMipmapMode TranslateMipFilterToVulkan(TextureFilter f) {
		switch (f) {
		case TextureFilter::Nearest:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case TextureFilter::Linear:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

		assert(false && "TranslateMipFilterToVulkan: Invalid TextureFilter!");
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;

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
			return ColorFormat::RG8;
		case VK_FORMAT_B8G8R8_UINT:
			return ColorFormat::RGB8;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_UNORM:
			return ColorFormat::RGBA8;
		}

		assert(false && "TranslateColorFormatFromVulkan: Invalid color format!");
		return ColorFormat::RGBA8;
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
		case ColorFormat::RG8:
			channels = 2;
			return VK_FORMAT_R8G8_UNORM;
		case ColorFormat::RGB8:
			channels = 3;
			return VK_FORMAT_B8G8R8_UNORM;
		case ColorFormat::RGBA8:
			channels = 4;
			return VK_FORMAT_B8G8R8A8_UNORM;

		case ColorFormat::R10G10B10A2:
			channels = 4;
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;

		case ColorFormat::R16:
			channels = 1;
			return VK_FORMAT_R16_SFLOAT;
		case ColorFormat::RG16:
			channels = 2;
			return VK_FORMAT_R16G16_SFLOAT;
		case ColorFormat::RGB16:
			channels = 3;
			return VK_FORMAT_R16G16B16_SFLOAT;
		case ColorFormat::RGBA16:
			channels = 4;
			return VK_FORMAT_R16G16B16A16_SFLOAT;

		case ColorFormat::R32:
			channels = 1;
			return VK_FORMAT_R32_SFLOAT;
		case ColorFormat::RG32:
			channels = 2;
			return VK_FORMAT_R32G32_SFLOAT;
		case ColorFormat::RGB32:
			channels = 3;
			return VK_FORMAT_R32G32B32_SFLOAT;
		case ColorFormat::RGBA32:
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
		case ColorFormat::BC6H:
			channels = 3;
			return VK_FORMAT_BC6H_UFLOAT_BLOCK;
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
		constexpr VkCullModeFlags cullModes[] = {
			VK_CULL_MODE_NONE,
			VK_CULL_MODE_FRONT_BIT,
			VK_CULL_MODE_BACK_BIT,
			VK_CULL_MODE_FRONT_AND_BACK,
		};

		return cullModes[static_cast<size_t>(cullMode)];
	}

	VkColorComponentFlags TranslateColorMaskToVulkan(ColorMask colorMask) {
		return
			(((colorMask & ColorMask::Red) == ColorMask::Red) ? VK_COLOR_COMPONENT_R_BIT : 0) |
			(((colorMask & ColorMask::Green) == ColorMask::Green) ? VK_COLOR_COMPONENT_G_BIT : 0) |
			(((colorMask & ColorMask::Blue) == ColorMask::Blue) ? VK_COLOR_COMPONENT_B_BIT : 0) |
			(((colorMask & ColorMask::Alpha) == ColorMask::Alpha) ? VK_COLOR_COMPONENT_A_BIT : 0);
	}

	VkPolygonMode TranslatePolygonModeToVulkan(PolygonFillMode mode) {
		constexpr VkPolygonMode polygonModes[] = {
			VK_POLYGON_MODE_POINT,
			VK_POLYGON_MODE_LINE,
			VK_POLYGON_MODE_FILL
		};

		return polygonModes[static_cast<size_t>(mode)];
	}

	VkPrimitiveTopology TranslateGeometryTypeToVulkan(GeometryType geometryType) {
		static VkPrimitiveTopology primTypes[] = {
			VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
			VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
			VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
			VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
		};

		return primTypes[static_cast<size_t>(geometryType)];
	}

	VkBlendOp TranslateBlendOpToVulkan(BlendOperation op) {
		constexpr VkBlendOp blendOps[] = {
			VK_BLEND_OP_MAX_ENUM,
			VK_BLEND_OP_ADD,
			VK_BLEND_OP_SUBTRACT,
			VK_BLEND_OP_REVERSE_SUBTRACT,
			VK_BLEND_OP_MIN,
			VK_BLEND_OP_MAX,
			VK_BLEND_OP_ZERO_EXT,
			VK_BLEND_OP_SRC_EXT,
			VK_BLEND_OP_DST_EXT,
			VK_BLEND_OP_SRC_OVER_EXT,
			VK_BLEND_OP_DST_OVER_EXT,
			VK_BLEND_OP_SRC_IN_EXT,
			VK_BLEND_OP_DST_IN_EXT,
			VK_BLEND_OP_SRC_OUT_EXT,
			VK_BLEND_OP_DST_OUT_EXT,
			VK_BLEND_OP_SRC_ATOP_EXT,
			VK_BLEND_OP_DST_ATOP_EXT,
			VK_BLEND_OP_XOR_EXT,
			VK_BLEND_OP_MULTIPLY_EXT,
			VK_BLEND_OP_SCREEN_EXT,
			VK_BLEND_OP_OVERLAY_EXT,
			VK_BLEND_OP_DARKEN_EXT,
			VK_BLEND_OP_LIGHTEN_EXT,
			VK_BLEND_OP_COLORDODGE_EXT,
			VK_BLEND_OP_COLORBURN_EXT,
			VK_BLEND_OP_HARDLIGHT_EXT,
			VK_BLEND_OP_SOFTLIGHT_EXT,
			VK_BLEND_OP_DIFFERENCE_EXT,
			VK_BLEND_OP_EXCLUSION_EXT,
			VK_BLEND_OP_INVERT_EXT,
			VK_BLEND_OP_INVERT_RGB_EXT,
			VK_BLEND_OP_LINEARDODGE_EXT,
			VK_BLEND_OP_LINEARBURN_EXT,
			VK_BLEND_OP_VIVIDLIGHT_EXT,
			VK_BLEND_OP_LINEARLIGHT_EXT,
			VK_BLEND_OP_PINLIGHT_EXT,
			VK_BLEND_OP_HARDMIX_EXT,
			VK_BLEND_OP_HSL_HUE_EXT,
			VK_BLEND_OP_HSL_SATURATION_EXT,
			VK_BLEND_OP_HSL_COLOR_EXT,
			VK_BLEND_OP_HSL_LUMINOSITY_EXT,
			VK_BLEND_OP_PLUS_EXT,
			VK_BLEND_OP_PLUS_CLAMPED_EXT,
			VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT,
			VK_BLEND_OP_PLUS_DARKER_EXT,
			VK_BLEND_OP_MINUS_EXT,
			VK_BLEND_OP_MINUS_CLAMPED_EXT,
			VK_BLEND_OP_CONTRAST_EXT,
			VK_BLEND_OP_INVERT_OVG_EXT,
			VK_BLEND_OP_RED_EXT,
			VK_BLEND_OP_GREEN_EXT,
			VK_BLEND_OP_BLUE_EXT
		};

		return blendOps[static_cast<size_t>(op)];
	}

	VkBlendFactor TranslateBlendFactorToVulkan(BlendFactor factor) {
		constexpr VkBlendFactor blendFactors[] = {
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_SRC_COLOR,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
			VK_BLEND_FACTOR_DST_COLOR,
			VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_FACTOR_DST_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
			VK_BLEND_FACTOR_CONSTANT_COLOR,
			VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
			VK_BLEND_FACTOR_CONSTANT_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
			VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
			VK_BLEND_FACTOR_SRC1_COLOR,
			VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
			VK_BLEND_FACTOR_SRC1_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
		};

		return blendFactors[static_cast<size_t>(factor)];
	}

	VkCompareOp TranslateCompareOpToVulkan(CompareOperation op) {
		constexpr VkCompareOp compareOps[] = {
			VK_COMPARE_OP_NEVER,
			VK_COMPARE_OP_LESS,
			VK_COMPARE_OP_EQUAL,
			VK_COMPARE_OP_LESS_OR_EQUAL,
			VK_COMPARE_OP_GREATER,
			VK_COMPARE_OP_NOT_EQUAL,
			VK_COMPARE_OP_GREATER_OR_EQUAL,
			VK_COMPARE_OP_ALWAYS
		};

		return compareOps[static_cast<size_t>(op)];
	}
}
