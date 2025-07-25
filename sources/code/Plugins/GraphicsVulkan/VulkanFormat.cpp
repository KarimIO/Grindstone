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

	VkFormat TranslateFormatToVulkan(Format format) {
		VkFormat formats[] = {
			VK_FORMAT_UNDEFINED,
			VK_FORMAT_R4G4_UNORM_PACK8,
			VK_FORMAT_R4G4B4A4_UNORM_PACK16,
			VK_FORMAT_B4G4R4A4_UNORM_PACK16,
			VK_FORMAT_R5G6B5_UNORM_PACK16,
			VK_FORMAT_B5G6R5_UNORM_PACK16,
			VK_FORMAT_R5G5B5A1_UNORM_PACK16,
			VK_FORMAT_B5G5R5A1_UNORM_PACK16,
			VK_FORMAT_A1R5G5B5_UNORM_PACK16,
			VK_FORMAT_R8_UNORM,
			VK_FORMAT_R8_SNORM,
			VK_FORMAT_R8_USCALED,
			VK_FORMAT_R8_SSCALED,
			VK_FORMAT_R8_UINT,
			VK_FORMAT_R8_SINT,
			VK_FORMAT_R8_SRGB,
			VK_FORMAT_R8G8_UNORM,
			VK_FORMAT_R8G8_SNORM,
			VK_FORMAT_R8G8_USCALED,
			VK_FORMAT_R8G8_SSCALED,
			VK_FORMAT_R8G8_UINT,
			VK_FORMAT_R8G8_SINT,
			VK_FORMAT_R8G8_SRGB,
			VK_FORMAT_R8G8B8_UNORM,
			VK_FORMAT_R8G8B8_SNORM,
			VK_FORMAT_R8G8B8_USCALED,
			VK_FORMAT_R8G8B8_SSCALED,
			VK_FORMAT_R8G8B8_UINT,
			VK_FORMAT_R8G8B8_SINT,
			VK_FORMAT_R8G8B8_SRGB,
			VK_FORMAT_B8G8R8_UNORM,
			VK_FORMAT_B8G8R8_SNORM,
			VK_FORMAT_B8G8R8_USCALED,
			VK_FORMAT_B8G8R8_SSCALED,
			VK_FORMAT_B8G8R8_UINT,
			VK_FORMAT_B8G8R8_SINT,
			VK_FORMAT_B8G8R8_SRGB,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_FORMAT_R8G8B8A8_SNORM,
			VK_FORMAT_R8G8B8A8_USCALED,
			VK_FORMAT_R8G8B8A8_SSCALED,
			VK_FORMAT_R8G8B8A8_UINT,
			VK_FORMAT_R8G8B8A8_SINT,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_B8G8R8A8_SNORM,
			VK_FORMAT_B8G8R8A8_USCALED,
			VK_FORMAT_B8G8R8A8_SSCALED,
			VK_FORMAT_B8G8R8A8_UINT,
			VK_FORMAT_B8G8R8A8_SINT,
			VK_FORMAT_B8G8R8A8_SRGB,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			VK_FORMAT_A8B8G8R8_SNORM_PACK32,
			VK_FORMAT_A8B8G8R8_USCALED_PACK32,
			VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
			VK_FORMAT_A8B8G8R8_UINT_PACK32,
			VK_FORMAT_A8B8G8R8_SINT_PACK32,
			VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			VK_FORMAT_A2R10G10B10_UNORM_PACK32,
			VK_FORMAT_A2R10G10B10_SNORM_PACK32,
			VK_FORMAT_A2R10G10B10_USCALED_PACK32,
			VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
			VK_FORMAT_A2R10G10B10_UINT_PACK32,
			VK_FORMAT_A2R10G10B10_SINT_PACK32,
			VK_FORMAT_A2B10G10R10_UNORM_PACK32,
			VK_FORMAT_A2B10G10R10_SNORM_PACK32,
			VK_FORMAT_A2B10G10R10_USCALED_PACK32,
			VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
			VK_FORMAT_A2B10G10R10_UINT_PACK32,
			VK_FORMAT_A2B10G10R10_SINT_PACK32,
			VK_FORMAT_R16_UNORM,
			VK_FORMAT_R16_SNORM,
			VK_FORMAT_R16_USCALED,
			VK_FORMAT_R16_SSCALED,
			VK_FORMAT_R16_UINT,
			VK_FORMAT_R16_SINT,
			VK_FORMAT_R16_SFLOAT,
			VK_FORMAT_R16G16_UNORM,
			VK_FORMAT_R16G16_SNORM,
			VK_FORMAT_R16G16_USCALED,
			VK_FORMAT_R16G16_SSCALED,
			VK_FORMAT_R16G16_UINT,
			VK_FORMAT_R16G16_SINT,
			VK_FORMAT_R16G16_SFLOAT,
			VK_FORMAT_R16G16B16_UNORM,
			VK_FORMAT_R16G16B16_SNORM,
			VK_FORMAT_R16G16B16_USCALED,
			VK_FORMAT_R16G16B16_SSCALED,
			VK_FORMAT_R16G16B16_UINT,
			VK_FORMAT_R16G16B16_SINT,
			VK_FORMAT_R16G16B16_SFLOAT,
			VK_FORMAT_R16G16B16A16_UNORM,
			VK_FORMAT_R16G16B16A16_SNORM,
			VK_FORMAT_R16G16B16A16_USCALED,
			VK_FORMAT_R16G16B16A16_SSCALED,
			VK_FORMAT_R16G16B16A16_UINT,
			VK_FORMAT_R16G16B16A16_SINT,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_FORMAT_R32_UINT,
			VK_FORMAT_R32_SINT,
			VK_FORMAT_R32_SFLOAT,
			VK_FORMAT_R32G32_UINT,
			VK_FORMAT_R32G32_SINT,
			VK_FORMAT_R32G32_SFLOAT,
			VK_FORMAT_R32G32B32_UINT,
			VK_FORMAT_R32G32B32_SINT,
			VK_FORMAT_R32G32B32_SFLOAT,
			VK_FORMAT_R32G32B32A32_UINT,
			VK_FORMAT_R32G32B32A32_SINT,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_FORMAT_R64_UINT,
			VK_FORMAT_R64_SINT,
			VK_FORMAT_R64_SFLOAT,
			VK_FORMAT_R64G64_UINT,
			VK_FORMAT_R64G64_SINT,
			VK_FORMAT_R64G64_SFLOAT,
			VK_FORMAT_R64G64B64_UINT,
			VK_FORMAT_R64G64B64_SINT,
			VK_FORMAT_R64G64B64_SFLOAT,
			VK_FORMAT_R64G64B64A64_UINT,
			VK_FORMAT_R64G64B64A64_SINT,
			VK_FORMAT_R64G64B64A64_SFLOAT,
			VK_FORMAT_B10G11R11_UFLOAT_PACK32,
			VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,

			VK_FORMAT_D16_UNORM,
			VK_FORMAT_X8_D24_UNORM_PACK32,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,

			VK_FORMAT_BC1_RGB_UNORM_BLOCK,
			VK_FORMAT_BC1_RGB_SRGB_BLOCK,
			VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
			VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
			VK_FORMAT_BC2_UNORM_BLOCK,
			VK_FORMAT_BC2_SRGB_BLOCK,
			VK_FORMAT_BC3_UNORM_BLOCK,
			VK_FORMAT_BC3_SRGB_BLOCK,
			VK_FORMAT_BC4_UNORM_BLOCK,
			VK_FORMAT_BC4_SNORM_BLOCK,
			VK_FORMAT_BC5_UNORM_BLOCK,
			VK_FORMAT_BC5_SNORM_BLOCK,
			VK_FORMAT_BC6H_UFLOAT_BLOCK,
			VK_FORMAT_BC6H_SFLOAT_BLOCK,
			VK_FORMAT_BC7_UNORM_BLOCK,
			VK_FORMAT_BC7_SRGB_BLOCK,
			VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
			VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
			VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
			VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
			VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
			VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
			VK_FORMAT_EAC_R11_UNORM_BLOCK,
			VK_FORMAT_EAC_R11_SNORM_BLOCK,
			VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
			VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
			VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
			VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
			VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
			VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
			VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
			VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
			VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
			VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
			VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
			VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
			VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
			VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
			VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
			VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
			VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
			VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
			VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
			VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
			VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
			VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
			VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
			VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
			VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
			VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
			VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
			VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
			VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
			VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
			VK_FORMAT_G8B8G8R8_422_UNORM,
			VK_FORMAT_B8G8R8G8_422_UNORM,
			VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
			VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
			VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
			VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
			VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
			VK_FORMAT_R10X6_UNORM_PACK16,
			VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
			VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
			VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
			VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
			VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
			VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
			VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
			VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
			VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
			VK_FORMAT_R12X4_UNORM_PACK16,
			VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
			VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
			VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
			VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
			VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
			VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
			VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
			VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
			VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
			VK_FORMAT_G16B16G16R16_422_UNORM,
			VK_FORMAT_B16G16R16G16_422_UNORM,
			VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
			VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
			VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
			VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
			VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
			VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
			VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
			VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
			VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
			VK_FORMAT_A4R4G4B4_UNORM_PACK16,
			VK_FORMAT_A4B4G4R4_UNORM_PACK16,
			VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
			VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
			VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
			VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
			VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
			VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
			VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
			VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
			VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
			VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
			
			// Limited Support:
			VK_FORMAT_R16G16_S10_5_NV,
			VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR,
			VK_FORMAT_A8_UNORM_KHR
		};

		return formats[static_cast<size_t>(format)];
	}

	Format TranslateFormatFromVulkan(VkFormat format) {
		Format formats[] = {
			Format::Invalid,
			Format::R4G4_UNORM_PACK8,
			Format::R4G4B4A4_UNORM_PACK16,
			Format::B4G4R4A4_UNORM_PACK16,
			Format::R5G6B5_UNORM_PACK16,
			Format::B5G6R5_UNORM_PACK16,
			Format::R5G5B5A1_UNORM_PACK16,
			Format::B5G5R5A1_UNORM_PACK16,
			Format::A1R5G5B5_UNORM_PACK16,
			Format::R8_UNORM,
			Format::R8_SNORM,
			Format::R8_USCALED,
			Format::R8_SSCALED,
			Format::R8_UINT,
			Format::R8_SINT,
			Format::R8_SRGB,
			Format::R8G8_UNORM,
			Format::R8G8_SNORM,
			Format::R8G8_USCALED,
			Format::R8G8_SSCALED,
			Format::R8G8_UINT,
			Format::R8G8_SINT,
			Format::R8G8_SRGB,
			Format::R8G8B8_UNORM,
			Format::R8G8B8_SNORM,
			Format::R8G8B8_USCALED,
			Format::R8G8B8_SSCALED,
			Format::R8G8B8_UINT,
			Format::R8G8B8_SINT,
			Format::R8G8B8_SRGB,
			Format::B8G8R8_UNORM,
			Format::B8G8R8_SNORM,
			Format::B8G8R8_USCALED,
			Format::B8G8R8_SSCALED,
			Format::B8G8R8_UINT,
			Format::B8G8R8_SINT,
			Format::B8G8R8_SRGB,
			Format::R8G8B8A8_UNORM,
			Format::R8G8B8A8_SNORM,
			Format::R8G8B8A8_USCALED,
			Format::R8G8B8A8_SSCALED,
			Format::R8G8B8A8_UINT,
			Format::R8G8B8A8_SINT,
			Format::R8G8B8A8_SRGB,
			Format::B8G8R8A8_UNORM,
			Format::B8G8R8A8_SNORM,
			Format::B8G8R8A8_USCALED,
			Format::B8G8R8A8_SSCALED,
			Format::B8G8R8A8_UINT,
			Format::B8G8R8A8_SINT,
			Format::B8G8R8A8_SRGB,
			Format::A8B8G8R8_UNORM_PACK32,
			Format::A8B8G8R8_SNORM_PACK32,
			Format::A8B8G8R8_USCALED_PACK32,
			Format::A8B8G8R8_SSCALED_PACK32,
			Format::A8B8G8R8_UINT_PACK32,
			Format::A8B8G8R8_SINT_PACK32,
			Format::A8B8G8R8_SRGB_PACK32,
			Format::A2R10G10B10_UNORM_PACK32,
			Format::A2R10G10B10_SNORM_PACK32,
			Format::A2R10G10B10_USCALED_PACK32,
			Format::A2R10G10B10_SSCALED_PACK32,
			Format::A2R10G10B10_UINT_PACK32,
			Format::A2R10G10B10_SINT_PACK32,
			Format::A2B10G10R10_UNORM_PACK32,
			Format::A2B10G10R10_SNORM_PACK32,
			Format::A2B10G10R10_USCALED_PACK32,
			Format::A2B10G10R10_SSCALED_PACK32,
			Format::A2B10G10R10_UINT_PACK32,
			Format::A2B10G10R10_SINT_PACK32,
			Format::R16_UNORM,
			Format::R16_SNORM,
			Format::R16_USCALED,
			Format::R16_SSCALED,
			Format::R16_UINT,
			Format::R16_SINT,
			Format::R16_SFLOAT,
			Format::R16G16_UNORM,
			Format::R16G16_SNORM,
			Format::R16G16_USCALED,
			Format::R16G16_SSCALED,
			Format::R16G16_UINT,
			Format::R16G16_SINT,
			Format::R16G16_SFLOAT,
			Format::R16G16B16_UNORM,
			Format::R16G16B16_SNORM,
			Format::R16G16B16_USCALED,
			Format::R16G16B16_SSCALED,
			Format::R16G16B16_UINT,
			Format::R16G16B16_SINT,
			Format::R16G16B16_SFLOAT,
			Format::R16G16B16A16_UNORM,
			Format::R16G16B16A16_SNORM,
			Format::R16G16B16A16_USCALED,
			Format::R16G16B16A16_SSCALED,
			Format::R16G16B16A16_UINT,
			Format::R16G16B16A16_SINT,
			Format::R16G16B16A16_SFLOAT,
			Format::R32_UINT,
			Format::R32_SINT,
			Format::R32_SFLOAT,
			Format::R32G32_UINT,
			Format::R32G32_SINT,
			Format::R32G32_SFLOAT,
			Format::R32G32B32_UINT,
			Format::R32G32B32_SINT,
			Format::R32G32B32_SFLOAT,
			Format::R32G32B32A32_UINT,
			Format::R32G32B32A32_SINT,
			Format::R32G32B32A32_SFLOAT,
			Format::R64_UINT,
			Format::R64_SINT,
			Format::R64_SFLOAT,
			Format::R64G64_UINT,
			Format::R64G64_SINT,
			Format::R64G64_SFLOAT,
			Format::R64G64B64_UINT,
			Format::R64G64B64_SINT,
			Format::R64G64B64_SFLOAT,
			Format::R64G64B64A64_UINT,
			Format::R64G64B64A64_SINT,
			Format::R64G64B64A64_SFLOAT,
			Format::B10G11R11_UFLOAT_PACK32,
			Format::E5B9G9R9_UFLOAT_PACK32,
			Format::D16_UNORM,
			Format::X8_D24_UNORM_PACK32,
			Format::D32_SFLOAT,
			Format::S8_UINT,
			Format::D16_UNORM_S8_UINT,
			Format::D24_UNORM_S8_UINT,
			Format::D32_SFLOAT_S8_UINT,
			Format::BC1_RGB_UNORM_BLOCK,
			Format::BC1_RGB_SRGB_BLOCK,
			Format::BC1_RGBA_UNORM_BLOCK,
			Format::BC1_RGBA_SRGB_BLOCK,
			Format::BC2_UNORM_BLOCK,
			Format::BC2_SRGB_BLOCK,
			Format::BC3_UNORM_BLOCK,
			Format::BC3_SRGB_BLOCK,
			Format::BC4_UNORM_BLOCK,
			Format::BC4_SNORM_BLOCK,
			Format::BC5_UNORM_BLOCK,
			Format::BC5_SNORM_BLOCK,
			Format::BC6H_UFLOAT_BLOCK,
			Format::BC6H_SFLOAT_BLOCK,
			Format::BC7_UNORM_BLOCK,
			Format::BC7_SRGB_BLOCK,
			Format::ETC2_R8G8B8_UNORM_BLOCK,
			Format::ETC2_R8G8B8_SRGB_BLOCK,
			Format::ETC2_R8G8B8A1_UNORM_BLOCK,
			Format::ETC2_R8G8B8A1_SRGB_BLOCK,
			Format::ETC2_R8G8B8A8_UNORM_BLOCK,
			Format::ETC2_R8G8B8A8_SRGB_BLOCK,
			Format::EAC_R11_UNORM_BLOCK,
			Format::EAC_R11_SNORM_BLOCK,
			Format::EAC_R11G11_UNORM_BLOCK,
			Format::EAC_R11G11_SNORM_BLOCK,
			Format::ASTC_4x4_UNORM_BLOCK,
			Format::ASTC_4x4_SRGB_BLOCK,
			Format::ASTC_5x4_UNORM_BLOCK,
			Format::ASTC_5x4_SRGB_BLOCK,
			Format::ASTC_5x5_UNORM_BLOCK,
			Format::ASTC_5x5_SRGB_BLOCK,
			Format::ASTC_6x5_UNORM_BLOCK,
			Format::ASTC_6x5_SRGB_BLOCK,
			Format::ASTC_6x6_UNORM_BLOCK,
			Format::ASTC_6x6_SRGB_BLOCK,
			Format::ASTC_8x5_UNORM_BLOCK,
			Format::ASTC_8x5_SRGB_BLOCK,
			Format::ASTC_8x6_UNORM_BLOCK,
			Format::ASTC_8x6_SRGB_BLOCK,
			Format::ASTC_8x8_UNORM_BLOCK,
			Format::ASTC_8x8_SRGB_BLOCK,
			Format::ASTC_10x5_UNORM_BLOCK,
			Format::ASTC_10x5_SRGB_BLOCK,
			Format::ASTC_10x6_UNORM_BLOCK,
			Format::ASTC_10x6_SRGB_BLOCK,
			Format::ASTC_10x8_UNORM_BLOCK,
			Format::ASTC_10x8_SRGB_BLOCK,
			Format::ASTC_10x10_UNORM_BLOCK,
			Format::ASTC_10x10_SRGB_BLOCK,
			Format::ASTC_12x10_UNORM_BLOCK,
			Format::ASTC_12x10_SRGB_BLOCK,
			Format::ASTC_12x12_UNORM_BLOCK,
			Format::ASTC_12x12_SRGB_BLOCK,
		};

		size_t formatAsSizeT = static_cast<size_t>(format);
		if (formatAsSizeT < static_cast<size_t>(VK_FORMAT_G8B8G8R8_422_UNORM)) {
			return formats[formatAsSizeT];
		}

		switch (format) {
		case VK_FORMAT_G8B8G8R8_422_UNORM: return Format::G8B8G8R8_422_UNORM;
		case VK_FORMAT_B8G8R8G8_422_UNORM: return Format::B8G8R8G8_422_UNORM;
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return Format::G8_B8_R8_3PLANE_420_UNORM;
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: return Format::G8_B8R8_2PLANE_420_UNORM;
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM: return Format::G8_B8_R8_3PLANE_422_UNORM;
		case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: return Format::G8_B8R8_2PLANE_422_UNORM;
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: return Format::G8_B8_R8_3PLANE_444_UNORM;
		case VK_FORMAT_R10X6_UNORM_PACK16: return Format::R10X6_UNORM_PACK16;
		case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: return Format::R10X6G10X6_UNORM_2PACK16;
		case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16;
		case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
		case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: return Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: return Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: return Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16;
		case VK_FORMAT_R12X4_UNORM_PACK16: return Format::R12X4_UNORM_PACK16;
		case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: return Format::R12X4G12X4_UNORM_2PACK16;
		case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: return Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16;
		case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;
		case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16;
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: return Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: return Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: return Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16;
		case VK_FORMAT_G16B16G16R16_422_UNORM: return Format::G16B16G16R16_422_UNORM;
		case VK_FORMAT_B16G16R16G16_422_UNORM: return Format::B16G16R16G16_422_UNORM;
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM: return Format::G16_B16_R16_3PLANE_420_UNORM;
		case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: return Format::G16_B16R16_2PLANE_420_UNORM;
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM: return Format::G16_B16_R16_3PLANE_422_UNORM;
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: return Format::G16_B16R16_2PLANE_422_UNORM;
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return Format::G16_B16_R16_3PLANE_444_UNORM;
		case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM: return Format::G8_B8R8_2PLANE_444_UNORM;
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: return Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16;
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: return Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16;
		case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM: return Format::G16_B16R16_2PLANE_444_UNORM;
		case VK_FORMAT_A4R4G4B4_UNORM_PACK16: return Format::A4R4G4B4_UNORM_PACK16;
		case VK_FORMAT_A4B4G4R4_UNORM_PACK16: return Format::A4B4G4R4_UNORM_PACK16;
		case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK: return Format::ASTC_4x4_SFLOAT;
		case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK: return Format::ASTC_5x4_SFLOAT;
		case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK: return Format::ASTC_5x5_SFLOAT;
		case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK: return Format::ASTC_6x5_SFLOAT;
		case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK: return Format::ASTC_6x6_SFLOAT;
		case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK: return Format::ASTC_8x5_SFLOAT;
		case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK: return Format::ASTC_8x6_SFLOAT;
		case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK: return Format::ASTC_8x8_SFLOAT;
		case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK: return Format::ASTC_10x5_SFLOAT;
		case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK: return Format::ASTC_10x6_SFLOAT;
		case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK: return Format::ASTC_10x8_SFLOAT;
		case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK: return Format::ASTC_10x10_SFLOAT;
		case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK: return Format::ASTC_12x10_SFLOAT;
		case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK: return Format::ASTC_12x12_SFLOAT;
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: return Format::PVRTC1_2BPP_UNORM;
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: return Format::PVRTC1_4BPP_UNORM;
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: return Format::PVRTC2_2BPP_UNORM;
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: return Format::PVRTC2_4BPP_UNORM;
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return Format::PVRTC1_2BPP_SRGB;
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return Format::PVRTC1_4BPP_SRGB;
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return Format::PVRTC2_2BPP_SRGB;
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return Format::PVRTC2_4BPP_SRGB;
		case VK_FORMAT_R16G16_S10_5_NV: return Format::R16G16_S10_5;
		case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR: return Format::A1B5G5R5_UNORM_PACK16;
		case VK_FORMAT_A8_UNORM_KHR: return Format::A8_UNORM;
		}

		return Format::Invalid;
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

	VkImageLayout TranslateImageLayoutToVulkan(Grindstone::GraphicsAPI::ImageLayout layout) {
		switch (layout) {
			case Grindstone::GraphicsAPI::ImageLayout::Undefined:			return VK_IMAGE_LAYOUT_UNDEFINED;
			case Grindstone::GraphicsAPI::ImageLayout::General:				return VK_IMAGE_LAYOUT_GENERAL;
			case Grindstone::GraphicsAPI::ImageLayout::ColorAttachment:		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case Grindstone::GraphicsAPI::ImageLayout::DepthWrite:			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case Grindstone::GraphicsAPI::ImageLayout::DepthRead:			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			case Grindstone::GraphicsAPI::ImageLayout::ShaderRead:			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case Grindstone::GraphicsAPI::ImageLayout::TransferSrc:			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case Grindstone::GraphicsAPI::ImageLayout::TransferDst:			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case Grindstone::GraphicsAPI::ImageLayout::Present:				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			default:														return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VkBlendOp TranslateBlendOpToVulkan(BlendOperation op) {
		constexpr VkBlendOp blendOps[] = {
			VK_BLEND_OP_ADD,
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
