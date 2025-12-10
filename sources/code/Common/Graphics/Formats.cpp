#include "Formats.hpp"

using namespace Grindstone::GraphicsAPI;

FormatDepthStencilType Grindstone::GraphicsAPI::GetFormatDepthStencilType(Format format) {
	switch (format) {
	case Format::D16_UNORM: return FormatDepthStencilType::NotDepthStencil;
	case Format::X8_D24_UNORM_PACK32: return FormatDepthStencilType::NotDepthStencil;
	case Format::D32_SFLOAT: return FormatDepthStencilType::Depth;
	case Format::S8_UINT: return FormatDepthStencilType::Stencil;
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

uint8_t Grindstone::GraphicsAPI::GetFormatBytesPerPixel(Grindstone::GraphicsAPI::Format format) {
	constexpr uint32_t formatSizeInBytes[] = {
		0, // Invalid
		1, // R4G4_UNORM_PACK8
		2, // R4G4B4A4_UNORM_PACK16
		2, // B4G4R4A4_UNORM_PACK16
		2, // R5G6B5_UNORM_PACK16
		2, // B5G6R5_UNORM_PACK16
		2, // R5G5B5A1_UNORM_PACK16
		2, // B5G5R5A1_UNORM_PACK16
		2, // A1R5G5B5_UNORM_PACK16
		1, // R8_UNORM
		1, // R8_SNORM
		1, // R8_USCALED
		1, // R8_SSCALED
		1, // R8_UINT
		1, // R8_SINT
		1, // R8_SRGB
		2, // R8G8_UNORM
		2, // R8G8_SNORM
		2, // R8G8_USCALED
		2, // R8G8_SSCALED
		2, // R8G8_UINT
		2, // R8G8_SINT
		2, // R8G8_SRGB
		3, // R8G8B8_UNORM
		3, // R8G8B8_SNORM
		3, // R8G8B8_USCALED
		3, // R8G8B8_SSCALED
		3, // R8G8B8_UINT
		3, // R8G8B8_SINT
		3, // R8G8B8_SRGB
		3, // B8G8R8_UNORM
		3, // B8G8R8_SNORM
		3, // B8G8R8_USCALED
		3, // B8G8R8_SSCALED
		3, // B8G8R8_UINT
		3, // B8G8R8_SINT
		3, // B8G8R8_SRGB
		4, // R8G8B8A8_UNORM
		4, // R8G8B8A8_SNORM
		4, // R8G8B8A8_USCALED
		4, // R8G8B8A8_SSCALED
		4, // R8G8B8A8_UINT
		4, // R8G8B8A8_SINT
		4, // R8G8B8A8_SRGB
		4, // B8G8R8A8_UNORM
		4, // B8G8R8A8_SNORM
		4, // B8G8R8A8_USCALED
		4, // B8G8R8A8_SSCALED
		4, // B8G8R8A8_UINT
		4, // B8G8R8A8_SINT
		4, // B8G8R8A8_SRGB
		4, // A8B8G8R8_UNORM_PACK32
		4, // A8B8G8R8_SNORM_PACK32
		4, // A8B8G8R8_USCALED_PACK32
		4, // A8B8G8R8_SSCALED_PACK32
		4, // A8B8G8R8_UINT_PACK32
		4, // A8B8G8R8_SINT_PACK32
		4, // A8B8G8R8_SRGB_PACK32
		4, // A2R10G10B10_UNORM_PACK32
		4, // A2R10G10B10_SNORM_PACK32
		4, // A2R10G10B10_USCALED_PACK32
		4, // A2R10G10B10_SSCALED_PACK32
		4, // A2R10G10B10_UINT_PACK32
		4, // A2R10G10B10_SINT_PACK32
		4, // A2B10G10R10_UNORM_PACK32
		4, // A2B10G10R10_SNORM_PACK32
		4, // A2B10G10R10_USCALED_PACK32
		4, // A2B10G10R10_SSCALED_PACK32
		4, // A2B10G10R10_UINT_PACK32
		4, // A2B10G10R10_SINT_PACK32
		2, // R16_UNORM
		2, // R16_SNORM
		2, // R16_USCALED
		2, // R16_SSCALED
		2, // R16_UINT
		2, // R16_SINT
		2, // R16_SFLOAT
		4, // R16G16_UNORM
		4, // R16G16_SNORM
		4, // R16G16_USCALED
		4, // R16G16_SSCALED
		4, // R16G16_UINT
		4, // R16G16_SINT
		4, // R16G16_SFLOAT
		6, // R16G16B16_UNORM
		6, // R16G16B16_SNORM
		6, // R16G16B16_USCALED
		6, // R16G16B16_SSCALED
		6, // R16G16B16_UINT
		6, // R16G16B16_SINT
		6, // R16G16B16_SFLOAT
		8, // R16G16B16A16_UNORM
		8, // R16G16B16A16_SNORM
		8, // R16G16B16A16_USCALED
		8, // R16G16B16A16_SSCALED
		8, // R16G16B16A16_UINT
		8, // R16G16B16A16_SINT
		8, // R16G16B16A16_SFLOAT
		4, // R32_UINT
		4, // R32_SINT
		4, // R32_SFLOAT
		8, // R32G32_UINT
		8, // R32G32_SINT
		8, // R32G32_SFLOAT
		12, // R32G32B32_UINT
		12, // R32G32B32_SINT
		12, // R32G32B32_SFLOAT
		16, // R32G32B32A32_UINT
		16, // R32G32B32A32_SINT
		16, // R32G32B32A32_SFLOAT
		8, // R64_UINT
		8, // R64_SINT
		8, // R64_SFLOAT
		16, // R64G64_UINT
		16, // R64G64_SINT
		16, // R64G64_SFLOAT
		24, // R64G64B64_UINT
		24, // R64G64B64_SINT
		24, // R64G64B64_SFLOAT
		32, // R64G64B64A64_UINT
		32, // R64G64B64A64_SINT
		32, // R64G64B64A64_SFLOAT
		4, // B10G11R11_UFLOAT_PACK32
		4, // E5B9G9R9_UFLOAT_PACK32
		2, // D16_UNORM
		4, // X8_D24_UNORM_PACK32
		4, // D32_SFLOAT
		1, // S8_UINT
		3, // D16_UNORM_S8_UINT (2+1, may include padding)
		4, // D24_UNORM_S8_UINT (3+1 packed)
		5, // D32_SFLOAT_S8_UINT (4+1, may include padding)
		8, // BC1_RGB_UNORM_BLOCK (4x4 block = 8 bytes)
		8, // BC1_RGB_SRGB_BLOCK
		8, // BC1_RGBA_UNORM_BLOCK
		8, // BC1_RGBA_SRGB_BLOCK
		16, // BC2_UNORM_BLOCK
		16, // BC2_SRGB_BLOCK
		16, // BC3_UNORM_BLOCK
		16, // BC3_SRGB_BLOCK
		8, // BC4_UNORM_BLOCK
		8, // BC4_SNORM_BLOCK
		16, // BC5_UNORM_BLOCK
		16, // BC5_SNORM_BLOCK
		16, // BC6H_UFLOAT_BLOCK
		16, // BC6H_SFLOAT_BLOCK
		16, // BC7_UNORM_BLOCK
		16, // BC7_SRGB_BLOCK
		8, // ETC2_R8G8B8_UNORM_BLOCK
		8, // ETC2_R8G8B8_SRGB_BLOCK
		8, // ETC2_R8G8B8A1_UNORM_BLOCK
		8, // ETC2_R8G8B8A1_SRGB_BLOCK
		16, // ETC2_R8G8B8A8_UNORM_BLOCK
		16, // ETC2_R8G8B8A8_SRGB_BLOCK
		8, // EAC_R11_UNORM_BLOCK
		8, // EAC_R11_SNORM_BLOCK
		16, // EAC_R11G11_UNORM_BLOCK
		16, // EAC_R11G11_SNORM_BLOCK
		16, // ASTC_4x4_UNORM_BLOCK
		16, // ASTC_4x4_SRGB_BLOCK
		16, // ASTC_5x4_UNORM_BLOCK
		16, // ASTC_5x4_SRGB_BLOCK
		16, // ASTC_5x5_UNORM_BLOCK
		16, // ASTC_5x5_SRGB_BLOCK
		16, // ASTC_6x5_UNORM_BLOCK
		16, // ASTC_6x5_SRGB_BLOCK
		16, // ASTC_6x6_UNORM_BLOCK
		16, // ASTC_6x6_SRGB_BLOCK
		16, // ASTC_8x5_UNORM_BLOCK
		16, // ASTC_8x5_SRGB_BLOCK
		16, // ASTC_8x6_UNORM_BLOCK
		16, // ASTC_8x6_SRGB_BLOCK
		16, // ASTC_8x8_UNORM_BLOCK
		16, // ASTC_8x8_SRGB_BLOCK
		16, // ASTC_10x5_UNORM_BLOCK
		16, // ASTC_10x5_SRGB_BLOCK
		16, // ASTC_10x6_UNORM_BLOCK
		16, // ASTC_10x6_SRGB_BLOCK
		16, // ASTC_10x8_UNORM_BLOCK
		16, // ASTC_10x8_SRGB_BLOCK
		16, // ASTC_10x10_UNORM_BLOCK
		16, // ASTC_10x10_SRGB_BLOCK
		16, // ASTC_12x10_UNORM_BLOCK
		16, // ASTC_12x10_SRGB_BLOCK
		16, // ASTC_12x12_UNORM_BLOCK
		16, // ASTC_12x12_SRGB_BLOCK

		3, // G8B8G8R8_422_UNORM (4:2:2 format, 3 channels)
		3, // B8G8R8G8_422_UNORM (4:2:2 format, 3 channels)
		3, // G8_B8_R8_3PLANE_420_UNORM (3 planes, 1 byte per plane)
		3, // G8_B8R8_2PLANE_420_UNORM (2 planes, 1 byte per plane)
		3, // G8_B8_R8_3PLANE_422_UNORM (3 planes, 1 byte per plane)
		3, // G8_B8R8_2PLANE_422_UNORM (2 planes, 1 byte per plane)
		3, // G8_B8_R8_3PLANE_444_UNORM (3 planes, 1 byte per plane)
		2, // R10X6_UNORM_PACK16 (1 pixel in 2 bytes, 10 bits per channel)
		4, // R10X6G10X6_UNORM_2PACK16 (2 channels in 4 bytes, 10 bits per channel)
		8, // R10X6G10X6B10X6A10X6_UNORM_4PACK16 (4 channels in 8 bytes, 10 bits per channel)
		8, // G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 (4 channels in 8 bytes, 10 bits per channel, 4:2:2)
		8, // B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 (4 channels in 8 bytes, 10 bits per channel, 4:2:2)
		6, // G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 (3 planes, 3 bytes total, 10 bits per channel)
		6, // G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 (2 planes, 3 bytes total, 10 bits per channel)
		6, // G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 (3 planes, 3 bytes total, 10 bits per channel)
		6, // G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 (2 planes, 3 bytes total, 10 bits per channel)
		6, // G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 (3 planes, 3 bytes total, 10 bits per channel)
		3, // R12X4_UNORM_PACK16 (1 pixel in 2 bytes, 12 bits per channel)
		6, // R12X4G12X4_UNORM_2PACK16 (2 channels in 3 bytes, 12 bits per channel)
		12, // R12X4G12X4B12X4A12X4_UNORM_4PACK16 (4 channels in 6 bytes, 12 bits per channel)
		12, // G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 (4 channels in 6 bytes, 12 bits per channel, 4:2:2)
		12, // B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 (4 channels in 6 bytes, 12 bits per channel, 4:2:2)
		9, // G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 (3 planes, 4 bytes total, 12 bits per channel)
		9, // G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 (2 planes, 4 bytes total, 12 bits per channel)
		9, // G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 (3 planes, 4 bytes total, 12 bits per channel)
		9, // G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 (2 planes, 4 bytes total, 12 bits per channel)
		9, // G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 (3 planes, 4 bytes total, 12 bits per channel)
		8, // G16B16G16R16_422_UNORM (4:2:2 format, 8 bytes total)
		8, // B16G16R16G16_422_UNORM (4:2:2 format, 8 bytes total)
		6, // G16_B16_R16_3PLANE_420_UNORM (3 planes, 2 bytes per plane)
		6, // G16_B16R16_2PLANE_420_UNORM (2 planes, 2 bytes per plane)
		6, // G16_B16_R16_3PLANE_422_UNORM (3 planes, 2 bytes per plane)
		6, // G16_B16R16_2PLANE_422_UNORM (2 planes, 2 bytes per plane)
		6, // G16_B16_R16_3PLANE_444_UNORM (3 planes, 2 bytes per plane)
		3, // G8_B8R8_2PLANE_444_UNORM (2 planes, 1 byte per plane)
		6, // G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 (2 planes, 3 bytes total, 10 bits per channel)
		9, // G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 (2 planes, 4 bytes total, 12 bits per channel)
		6, // G16_B16R16_2PLANE_444_UNORM (2 planes, 2 bytes per plane)
		2, // A4R4G4B4_UNORM_PACK16 (1 pixel in 2 bytes)
		2, // A4B4G4R4_UNORM_PACK16 (1 pixel in 2 bytes)
		8, // ASTC_4x4_SFLOAT (4x4 block, 8 bytes total)
		8, // ASTC_5x4_SFLOAT (5x4 block, 8 bytes total)
		8, // ASTC_5x5_SFLOAT (5x5 block, 8 bytes total)
		8, // ASTC_6x5_SFLOAT (6x5 block, 8 bytes total)
		8, // ASTC_6x6_SFLOAT (6x6 block, 8 bytes total)
		8, // ASTC_8x5_SFLOAT (8x5 block, 8 bytes total)
		8, // ASTC_8x6_SFLOAT (8x6 block, 8 bytes total)
		8, // ASTC_8x8_SFLOAT (8x8 block, 8 bytes total)
		8, // ASTC_10x5_SFLOAT (10x5 block, 8 bytes total)
		8, // ASTC_10x6_SFLOAT (10x6 block, 8 bytes total)
		8, // ASTC_10x8_SFLOAT (10x8 block, 8 bytes total)
		8, // ASTC_10x10_SFLOAT (10x10 block, 8 bytes total)
		8, // ASTC_12x10_SFLOAT (12x10 block, 8 bytes total)
		8, // ASTC_12x12_SFLOAT (12x12 block, 8 bytes total)
		1, // PVRTC1_2BPP_UNORM (2 bits per pixel, 1 byte per 4 pixels)
		2, // PVRTC1_4BPP_UNORM (4 bits per pixel, 1 byte per 2 pixels)
		1, // PVRTC2_2BPP_UNORM (2 bits per pixel, 1 byte per 4 pixels)
		2, // PVRTC2_4BPP_UNORM (4 bits per pixel, 1 byte per 2 pixels)
		1, // PVRTC1_2BPP_SRGB (2 bits per pixel, 1 byte per 4 pixels)
		2, // PVRTC1_4BPP_SRGB (4 bits per pixel, 1 byte per 2 pixels)
		1, // PVRTC2_2BPP_SRGB (2 bits per pixel, 1 byte per 4 pixels)
		2, // PVRTC2_4BPP_SRGB (4 bits per pixel, 1 byte per 2 pixels)
		2, // R16G16_S10_5 (5 bits per channel, 2 bytes total)
		2, // A1B5G5R5_UNORM_PACK16 (1 pixel in 2 bytes)
		1, // A8_UNORM (1 byte per pixel)
	};

	return formatSizeInBytes[static_cast<size_t>(format)];
}
