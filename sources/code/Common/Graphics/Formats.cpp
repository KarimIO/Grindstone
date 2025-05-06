#include "Formats.hpp"

using namespace Grindstone::GraphicsAPI;

FormatDepthStencilType Grindstone::GraphicsAPI::GetFormatDepthStencilType(Format format) {
	switch (format) {
	case Format::D16_UNORM: return FormatDepthStencilType::NotDepthStencil;
	case Format::X8_D24_UNORM_PACK32: return FormatDepthStencilType::NotDepthStencil;
	case Format::D32_SFLOAT: return FormatDepthStencilType::DepthOnly;
	case Format::S8_UINT: return FormatDepthStencilType::StencilOnly;
	case Format::D16_UNORM_S8_UINT: return FormatDepthStencilType::DepthStencil;
	case Format::D24_UNORM_S8_UINT: return FormatDepthStencilType::DepthStencil;
	case Format::D32_SFLOAT_S8_UINT: return FormatDepthStencilType::DepthStencil;
	default: return FormatDepthStencilType::NotDepthStencil;
	}
}

bool Grindstone::GraphicsAPI::IsFormatCompressed(Format format) {
	switch (format) {
	case Format::BC1_RGB_UNORM_BLOCK: return true;
	case Format::BC1_RGB_SRGB_BLOCK: return true;
	case Format::BC1_RGBA_UNORM_BLOCK: return true;
	case Format::BC1_RGBA_SRGB_BLOCK: return true;
	case Format::BC2_UNORM_BLOCK: return true;
	case Format::BC2_SRGB_BLOCK: return true;
	case Format::BC3_UNORM_BLOCK: return true;
	case Format::BC3_SRGB_BLOCK: return true;
	case Format::BC4_UNORM_BLOCK: return true;
	case Format::BC4_SNORM_BLOCK: return true;
	case Format::BC5_UNORM_BLOCK: return true;
	case Format::BC5_SNORM_BLOCK: return true;
	case Format::BC6H_UFLOAT_BLOCK: return true;
	case Format::BC6H_SFLOAT_BLOCK: return true;
	case Format::BC7_UNORM_BLOCK: return true;
	case Format::BC7_SRGB_BLOCK: return true;
	case Format::ETC2_R8G8B8_UNORM_BLOCK: return true;
	case Format::ETC2_R8G8B8_SRGB_BLOCK: return true;
	case Format::ETC2_R8G8B8A1_UNORM_BLOCK: return true;
	case Format::ETC2_R8G8B8A1_SRGB_BLOCK: return true;
	case Format::ETC2_R8G8B8A8_UNORM_BLOCK: return true;
	case Format::ETC2_R8G8B8A8_SRGB_BLOCK: return true;
	case Format::EAC_R11_UNORM_BLOCK: return true;
	case Format::EAC_R11_SNORM_BLOCK: return true;
	case Format::EAC_R11G11_UNORM_BLOCK: return true;
	case Format::EAC_R11G11_SNORM_BLOCK: return true;
	case Format::ASTC_4x4_UNORM_BLOCK: return true;
	case Format::ASTC_4x4_SRGB_BLOCK: return true;
	case Format::ASTC_5x4_UNORM_BLOCK: return true;
	case Format::ASTC_5x4_SRGB_BLOCK: return true;
	case Format::ASTC_5x5_UNORM_BLOCK: return true;
	case Format::ASTC_5x5_SRGB_BLOCK: return true;
	case Format::ASTC_6x5_UNORM_BLOCK: return true;
	case Format::ASTC_6x5_SRGB_BLOCK: return true;
	case Format::ASTC_6x6_UNORM_BLOCK: return true;
	case Format::ASTC_6x6_SRGB_BLOCK: return true;
	case Format::ASTC_8x5_UNORM_BLOCK: return true;
	case Format::ASTC_8x5_SRGB_BLOCK: return true;
	case Format::ASTC_8x6_UNORM_BLOCK: return true;
	case Format::ASTC_8x6_SRGB_BLOCK: return true;
	case Format::ASTC_8x8_UNORM_BLOCK: return true;
	case Format::ASTC_8x8_SRGB_BLOCK: return true;
	case Format::ASTC_10x5_UNORM_BLOCK: return true;
	case Format::ASTC_10x5_SRGB_BLOCK: return true;
	case Format::ASTC_10x6_UNORM_BLOCK: return true;
	case Format::ASTC_10x6_SRGB_BLOCK: return true;
	case Format::ASTC_10x8_UNORM_BLOCK: return true;
	case Format::ASTC_10x8_SRGB_BLOCK: return true;
	case Format::ASTC_10x10_UNORM_BLOCK: return true;
	case Format::ASTC_10x10_SRGB_BLOCK: return true;
	case Format::ASTC_12x10_UNORM_BLOCK: return true;
	case Format::ASTC_12x10_SRGB_BLOCK: return true;
	case Format::ASTC_12x12_UNORM_BLOCK: return true;
	case Format::ASTC_12x12_SRGB_BLOCK: return true;
	case Format::ASTC_4x4_SFLOAT: return true;
	case Format::ASTC_5x4_SFLOAT: return true;
	case Format::ASTC_5x5_SFLOAT: return true;
	case Format::ASTC_6x5_SFLOAT: return true;
	case Format::ASTC_6x6_SFLOAT: return true;
	case Format::ASTC_8x5_SFLOAT: return true;
	case Format::ASTC_8x6_SFLOAT: return true;
	case Format::ASTC_8x8_SFLOAT: return true;
	case Format::ASTC_10x5_SFLOAT: return true;
	case Format::ASTC_10x6_SFLOAT: return true;
	case Format::ASTC_10x8_SFLOAT: return true;
	case Format::ASTC_10x10_SFLOAT: return true;
	case Format::ASTC_12x10_SFLOAT: return true;
	case Format::ASTC_12x12_SFLOAT: return true;
	case Format::PVRTC1_2BPP_UNORM: return true;
	case Format::PVRTC1_4BPP_UNORM: return true;
	case Format::PVRTC2_2BPP_UNORM: return true;
	case Format::PVRTC2_4BPP_UNORM: return true;
	case Format::PVRTC1_2BPP_SRGB: return true;
	case Format::PVRTC1_4BPP_SRGB: return true;
	case Format::PVRTC2_2BPP_SRGB: return true;
	case Format::PVRTC2_4BPP_SRGB: return true;
	}

	return false;
}

uint8_t Grindstone::GraphicsAPI::GetCompressedFormatBlockSize(Format format) {
	switch (format) {
	case Format::BC1_RGB_UNORM_BLOCK: return 8;
	case Format::BC1_RGB_SRGB_BLOCK: return 8;
	case Format::BC1_RGBA_UNORM_BLOCK: return 8;
	case Format::BC1_RGBA_SRGB_BLOCK: return 8;
	case Format::BC2_UNORM_BLOCK: return 16;
	case Format::BC2_SRGB_BLOCK: return 16;
	case Format::BC3_UNORM_BLOCK: return 16;
	case Format::BC3_SRGB_BLOCK: return 16;
	case Format::BC4_UNORM_BLOCK: return 8;
	case Format::BC4_SNORM_BLOCK: return 8;
	case Format::BC5_UNORM_BLOCK: return 16;
	case Format::BC5_SNORM_BLOCK: return 16;
	case Format::BC6H_UFLOAT_BLOCK: return 16;
	case Format::BC6H_SFLOAT_BLOCK: return 16;
	case Format::BC7_UNORM_BLOCK: return 16;
	case Format::BC7_SRGB_BLOCK: return 16;
	case Format::ETC2_R8G8B8_UNORM_BLOCK: return 8;
	case Format::ETC2_R8G8B8_SRGB_BLOCK: return 8;
	case Format::ETC2_R8G8B8A1_UNORM_BLOCK: return 8;
	case Format::ETC2_R8G8B8A1_SRGB_BLOCK: return 8;
	case Format::ETC2_R8G8B8A8_UNORM_BLOCK: return 16;
	case Format::ETC2_R8G8B8A8_SRGB_BLOCK: return 16;
	case Format::EAC_R11_UNORM_BLOCK: return 8;
	case Format::EAC_R11_SNORM_BLOCK: return 8;
	case Format::EAC_R11G11_UNORM_BLOCK: return 16;
	case Format::EAC_R11G11_SNORM_BLOCK: return 16;
	case Format::ASTC_4x4_UNORM_BLOCK: return 16;
	case Format::ASTC_4x4_SRGB_BLOCK: return 16;
	case Format::ASTC_5x4_UNORM_BLOCK: return 16;
	case Format::ASTC_5x4_SRGB_BLOCK: return 16;
	case Format::ASTC_5x5_UNORM_BLOCK: return 16;
	case Format::ASTC_5x5_SRGB_BLOCK: return 16;
	case Format::ASTC_6x5_UNORM_BLOCK: return 16;
	case Format::ASTC_6x5_SRGB_BLOCK: return 16;
	case Format::ASTC_6x6_UNORM_BLOCK: return 16;
	case Format::ASTC_6x6_SRGB_BLOCK: return 16;
	case Format::ASTC_8x5_UNORM_BLOCK: return 16;
	case Format::ASTC_8x5_SRGB_BLOCK: return 16;
	case Format::ASTC_8x6_UNORM_BLOCK: return 16;
	case Format::ASTC_8x6_SRGB_BLOCK: return 16;
	case Format::ASTC_8x8_UNORM_BLOCK: return 16;
	case Format::ASTC_8x8_SRGB_BLOCK: return 16;
	case Format::ASTC_10x5_UNORM_BLOCK: return 16;
	case Format::ASTC_10x5_SRGB_BLOCK: return 16;
	case Format::ASTC_10x6_UNORM_BLOCK: return 16;
	case Format::ASTC_10x6_SRGB_BLOCK: return 16;
	case Format::ASTC_10x8_UNORM_BLOCK: return 16;
	case Format::ASTC_10x8_SRGB_BLOCK: return 16;
	case Format::ASTC_10x10_UNORM_BLOCK: return 16;
	case Format::ASTC_10x10_SRGB_BLOCK: return 16;
	case Format::ASTC_12x10_UNORM_BLOCK: return 16;
	case Format::ASTC_12x10_SRGB_BLOCK: return 16;
	case Format::ASTC_12x12_UNORM_BLOCK: return 16;
	case Format::ASTC_12x12_SRGB_BLOCK: return 16;
	case Format::ASTC_4x4_SFLOAT: return 16;
	case Format::ASTC_5x4_SFLOAT: return 16;
	case Format::ASTC_5x5_SFLOAT: return 16;
	case Format::ASTC_6x5_SFLOAT: return 16;
	case Format::ASTC_6x6_SFLOAT: return 16;
	case Format::ASTC_8x5_SFLOAT: return 16;
	case Format::ASTC_8x6_SFLOAT: return 16;
	case Format::ASTC_8x8_SFLOAT: return 16;
	case Format::ASTC_10x5_SFLOAT: return 16;
	case Format::ASTC_10x6_SFLOAT: return 16;
	case Format::ASTC_10x8_SFLOAT: return 16;
	case Format::ASTC_10x10_SFLOAT: return 16;
	case Format::ASTC_12x10_SFLOAT: return 16;
	case Format::ASTC_12x12_SFLOAT: return 16;
	case Format::PVRTC1_2BPP_UNORM: return 8;
	case Format::PVRTC1_4BPP_UNORM: return 8;
	case Format::PVRTC2_2BPP_UNORM: return 8;
	case Format::PVRTC2_4BPP_UNORM: return 8;
	case Format::PVRTC1_2BPP_SRGB: return 8;
	case Format::PVRTC1_4BPP_SRGB: return 8;
	case Format::PVRTC2_2BPP_SRGB: return 8;
	case Format::PVRTC2_4BPP_SRGB: return 8;
	}

	return 0;
}
