#include <EngineCore/Logger.hpp>

#include "DdsParser.hpp"

static Grindstone::GraphicsAPI::Format MapFourCC(DWORD fourcc, bool hasAlpha) {
	using Format = Grindstone::GraphicsAPI::Format;
	switch (fourcc) {
	case Grindstone::Formats::DDS::FOURCC_BC1_UNORM: return hasAlpha ? Format::BC1_RGBA_UNORM_BLOCK : Format::BC1_RGB_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC2_UNORM: return Format::BC2_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC3_UNORM: return Format::BC3_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC4_UNORM_ATI: return Format::BC4_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC4_UNORM: return Format::BC4_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC4_SNORM: return Format::BC4_SNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC5_UNORM_ATI: return Format::BC5_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC5_UNORM: return Format::BC5_UNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_BC5_SNORM: return Format::BC5_SNORM_BLOCK;
	case Grindstone::Formats::DDS::FOURCC_R16_SFLOAT: return Format::R16_SFLOAT;
	case Grindstone::Formats::DDS::FOURCC_R16G16_SFLOAT: return Format::R16G16_SFLOAT;
	case Grindstone::Formats::DDS::FOURCC_R16G16B16A16_SFLOAT: return Format::R16G16B16A16_SFLOAT;
	case Grindstone::Formats::DDS::FOURCC_R32_SFLOAT: return Format::R32_SFLOAT;
	case Grindstone::Formats::DDS::FOURCC_R32G32_SFLOAT: return Format::R32G32_SFLOAT;
	case Grindstone::Formats::DDS::FOURCC_R32G32B32A32_SFLOAT: return Format::R32G32B32A32_SFLOAT;

	case Grindstone::Formats::DDS::FOURCC_R8G8_B8G8_UNORM: return Format::G8B8G8R8_422_UNORM;
	case Grindstone::Formats::DDS::FOURCC_G8R8_G8B8_UNORM: return Format::B8G8R8G8_422_UNORM;
	case Grindstone::Formats::DDS::FOURCC_R16G16B16A16_UNORM: return Format::R16G16B16A16_UNORM;
	case Grindstone::Formats::DDS::FOURCC_R16G16B16A16_SNORM: return Format::R16G16B16A16_SNORM;

	case Grindstone::Formats::DDS::FOURCC_CxV8U8: return Format::Invalid;
	case Grindstone::Formats::DDS::FOURCC_DXT2_UNORM: return Format::Invalid;
	case Grindstone::Formats::DDS::FOURCC_DXT4_UNORM: return Format::Invalid;
	case Grindstone::Formats::DDS::FOURCC_UYVY: return Format::Invalid;
	case Grindstone::Formats::DDS::FOURCC_YUY2: return Format::Invalid;
	default: return Format::Invalid;
	}
}

static Grindstone::GraphicsAPI::Format MapDXGIToFormat(Grindstone::Formats::DDS::DxgiFormat dxgiFormat) {
	using Format = Grindstone::GraphicsAPI::Format;
	using Dxgi = Grindstone::Formats::DDS::DxgiFormat;
	switch (dxgiFormat) {
	case Dxgi::R32G32B32A32_TYPELESS: return Format::R32G32B32A32_SFLOAT;
	case Dxgi::R32G32B32A32_FLOAT: return Format::R32G32B32A32_SFLOAT;
	case Dxgi::R32G32B32A32_UINT: return Format::R32G32B32A32_UINT;
	case Dxgi::R32G32B32A32_SINT: return Format::R32G32B32A32_SINT;
	case Dxgi::R32G32B32_TYPELESS: return Format::R32G32B32_SFLOAT;
	case Dxgi::R32G32B32_FLOAT: return Format::R32G32B32_SFLOAT;
	case Dxgi::R32G32B32_UINT: return Format::R32G32B32_UINT;
	case Dxgi::R32G32B32_SINT: return Format::R32G32B32_SINT;
	case Dxgi::R16G16B16A16_TYPELESS: return Format::R16G16B16A16_SFLOAT;
	case Dxgi::R16G16B16A16_FLOAT: return Format::R16G16B16A16_SFLOAT;
	case Dxgi::R16G16B16A16_UNORM: return Format::R16G16B16A16_UNORM;
	case Dxgi::R16G16B16A16_UINT: return Format::R16G16B16A16_UINT;
	case Dxgi::R16G16B16A16_SNORM: return Format::R16G16B16A16_SNORM;
	case Dxgi::R16G16B16A16_SINT: return Format::R16G16B16A16_SINT;
	case Dxgi::R32G32_TYPELESS: return Format::R32G32_SFLOAT;
	case Dxgi::R32G32_FLOAT: return Format::R32G32_SFLOAT;
	case Dxgi::R32G32_UINT: return Format::R32G32_UINT;
	case Dxgi::R32G32_SINT: return Format::R32G32_SINT;
	case Dxgi::R32G8X24_TYPELESS: return Format::D32_SFLOAT_S8_UINT;
	case Dxgi::D32_FLOAT_S8X24_UINT: return Format::D32_SFLOAT_S8_UINT;
	case Dxgi::R32_FLOAT_X8X24_TYPELESS: return Format::D32_SFLOAT_S8_UINT;
	case Dxgi::X32_TYPELESS_G8X24_UINT: return Format::D32_SFLOAT_S8_UINT;
	case Dxgi::R10G10B10A2_TYPELESS: return Format::A2R10G10B10_UNORM_PACK32;
	case Dxgi::R10G10B10A2_UNORM: return Format::A2R10G10B10_UNORM_PACK32;
	case Dxgi::R10G10B10A2_UINT: return Format::A2R10G10B10_UINT_PACK32;
	case Dxgi::R11G11B10_FLOAT: return Format::B10G11R11_UFLOAT_PACK32;
	case Dxgi::R8G8B8A8_TYPELESS: return Format::R8G8B8A8_UNORM;
	case Dxgi::R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
	case Dxgi::R8G8B8A8_UNORM_SRGB: return Format::R8G8B8A8_SRGB;
	case Dxgi::R8G8B8A8_UINT: return Format::R8G8B8A8_UINT;
	case Dxgi::R8G8B8A8_SNORM: return Format::R8G8B8A8_SNORM;
	case Dxgi::R8G8B8A8_SINT: return Format::R8G8B8A8_SINT;
	case Dxgi::R16G16_TYPELESS: return Format::R16G16_SFLOAT;
	case Dxgi::R16G16_FLOAT: return Format::R16G16_SFLOAT;
	case Dxgi::R16G16_UNORM: return Format::R16G16_UNORM;
	case Dxgi::R16G16_UINT: return Format::R16G16_UINT;
	case Dxgi::R16G16_SNORM: return Format::R16G16_SNORM;
	case Dxgi::R16G16_SINT: return Format::R16G16_SINT;
	case Dxgi::R32_TYPELESS: return Format::D32_SFLOAT;
	case Dxgi::D32_FLOAT: return Format::D32_SFLOAT;
	case Dxgi::R32_FLOAT: return Format::R32_SFLOAT;
	case Dxgi::R32_UINT: return Format::R32_UINT;
	case Dxgi::R32_SINT: return Format::R32_SINT;
	case Dxgi::R24G8_TYPELESS: return Format::D24_UNORM_S8_UINT;
	case Dxgi::D24_UNORM_S8_UINT: return Format::D24_UNORM_S8_UINT;
	case Dxgi::R24_UNORM_X8_TYPELESS: return Format::X8_D24_UNORM_PACK32;
	case Dxgi::X24_TYPELESS_G8_UINT: return Format::X8_D24_UNORM_PACK32;
	case Dxgi::R8G8_TYPELESS: return Format::R8G8_SNORM;
	case Dxgi::R8G8_UNORM: return Format::R8G8_UNORM;
	case Dxgi::R8G8_UINT: return Format::R8G8_UINT;
	case Dxgi::R8G8_SNORM: return Format::R8G8_SNORM;
	case Dxgi::R8G8_SINT: return Format::R8G8_SINT;
	case Dxgi::R16_TYPELESS: return Format::R16_SFLOAT;
	case Dxgi::R16_FLOAT: return Format::R16_SFLOAT;
	case Dxgi::D16_UNORM: return Format::D16_UNORM;
	case Dxgi::R16_UNORM: return Format::R16_UNORM;
	case Dxgi::R16_UINT: return Format::R16_UINT;
	case Dxgi::R16_SNORM: return Format::R16_SNORM;
	case Dxgi::R16_SINT: return Format::R16_SINT;
	case Dxgi::R8_TYPELESS: return Format::R8_UNORM;
	case Dxgi::R8_UNORM: return Format::R8_UNORM;
	case Dxgi::R8_UINT: return Format::R8_UINT;
	case Dxgi::R8_SNORM: return Format::R8_SNORM;
	case Dxgi::R8_SINT: return Format::R8_SINT;
	case Dxgi::A8_UNORM: return Format::A8_UNORM;
	case Dxgi::R1_UNORM: return Format::Invalid;
	case Dxgi::R9G9B9E5_SHAREDEXP: return Format::E5B9G9R9_UFLOAT_PACK32;
	case Dxgi::R8G8_B8G8_UNORM: return Format::B8G8R8G8_422_UNORM;
	case Dxgi::G8R8_G8B8_UNORM: return Format::G8B8G8R8_422_UNORM;
	case Dxgi::BC1_TYPELESS: return Format::BC1_RGBA_UNORM_BLOCK;
	case Dxgi::BC1_UNORM: return Format::BC1_RGBA_UNORM_BLOCK;
	case Dxgi::BC1_UNORM_SRGB: return Format::BC1_RGB_SRGB_BLOCK;
	case Dxgi::BC2_TYPELESS: return Format::BC2_UNORM_BLOCK;
	case Dxgi::BC2_UNORM: return Format::BC2_UNORM_BLOCK;
	case Dxgi::BC2_UNORM_SRGB: return Format::BC2_UNORM_BLOCK;
	case Dxgi::BC3_TYPELESS: return Format::BC3_UNORM_BLOCK;
	case Dxgi::BC3_UNORM: return Format::BC3_UNORM_BLOCK;
	case Dxgi::BC3_UNORM_SRGB: return Format::BC3_SRGB_BLOCK;
	case Dxgi::BC4_TYPELESS: return Format::BC4_UNORM_BLOCK;
	case Dxgi::BC4_UNORM: return Format::BC4_UNORM_BLOCK;
	case Dxgi::BC4_SNORM: return Format::BC4_SNORM_BLOCK;
	case Dxgi::BC5_TYPELESS: return Format::BC5_UNORM_BLOCK;
	case Dxgi::BC5_UNORM: return Format::BC5_UNORM_BLOCK;
	case Dxgi::BC5_SNORM: return Format::BC5_SNORM_BLOCK;
	case Dxgi::B5G6R5_UNORM: return Format::B5G6R5_UNORM_PACK16;
	case Dxgi::B5G5R5A1_UNORM: return Format::B5G5R5A1_UNORM_PACK16;
	case Dxgi::B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
	case Dxgi::B8G8R8X8_UNORM: return Format::B8G8R8A8_UNORM;
	case Dxgi::R10G10B10_XR_BIAS_A2_UNORM: return Format::A2R10G10B10_UNORM_PACK32;
	case Dxgi::B8G8R8A8_TYPELESS: return Format::B8G8R8A8_UNORM;
	case Dxgi::B8G8R8A8_UNORM_SRGB: return Format::B8G8R8A8_UNORM;
	case Dxgi::B8G8R8X8_TYPELESS: return Format::B8G8R8A8_UNORM;
	case Dxgi::B8G8R8X8_UNORM_SRGB: return Format::B8G8R8A8_SRGB;
	case Dxgi::BC6H_TYPELESS: return Format::BC6H_UFLOAT_BLOCK;
	case Dxgi::BC6H_UF16: return Format::BC6H_UFLOAT_BLOCK;
	case Dxgi::BC6H_SF16: return Format::BC6H_SFLOAT_BLOCK;
	case Dxgi::BC7_TYPELESS: return Format::BC7_UNORM_BLOCK;
	case Dxgi::BC7_UNORM: return Format::BC7_UNORM_BLOCK;
	case Dxgi::BC7_UNORM_SRGB: return Format::BC7_SRGB_BLOCK;
	case Dxgi::AYUV: return Format::Invalid;
	case Dxgi::Y410: return Format::Invalid;
	case Dxgi::Y416: return Format::Invalid;
	case Dxgi::NV12: return Format::Invalid;
	case Dxgi::P010: return Format::Invalid;
	case Dxgi::P016: return Format::Invalid;
	case Dxgi::OPAQUE_420: return Format::Invalid;
	case Dxgi::YUY2: return Format::Invalid;
	case Dxgi::Y210: return Format::Invalid;
	case Dxgi::Y216: return Format::Invalid;
	case Dxgi::NV11: return Format::Invalid;
	case Dxgi::AI44: return Format::Invalid;
	case Dxgi::IA44: return Format::Invalid;
	case Dxgi::P8: return Format::Invalid;
	case Dxgi::A8P8: return Format::Invalid;
	case Dxgi::B4G4R4A4_UNORM: return Format::B4G4R4A4_UNORM_PACK16;
	case Dxgi::P208: return Format::Invalid;
	case Dxgi::V208: return Format::Invalid;
	case Dxgi::V408: return Format::Invalid;
	default: return Format::Invalid;
	}
}

static Grindstone::GraphicsAPI::Format DetectFormatFromPixelFormat(const Grindstone::Formats::DDS::DDS_PIXELFORMAT& pf) {
	using Format = Grindstone::GraphicsAPI::Format;
	using namespace Grindstone::Formats::DDS;

	// --- RGB / RGBA formats ---
	if (pf.dwFlags & DDPF_RGB) {
		switch (pf.dwRGBBitCount) {
		case 32:
			if (pf.dwRBitMask == 0x00ff0000 && pf.dwGBitMask == 0x0000ff00 &&
				pf.dwBBitMask == 0x000000ff && pf.dwABitMask == 0xff000000)
				return Format::R8G8B8A8_UNORM; // A8R8G8B8

			if (pf.dwRBitMask == 0x00ff0000 && pf.dwGBitMask == 0x0000ff00 &&
				pf.dwBBitMask == 0x000000ff && pf.dwABitMask == 0x00000000)
				return Format::R8G8B8A8_UNORM; // X8R8G8B8

			if (pf.dwRBitMask == 0x000000ff && pf.dwGBitMask == 0x0000ff00 &&
				pf.dwBBitMask == 0x00ff0000 && pf.dwABitMask == 0xff000000)
				return Format::A8B8G8R8_UNORM_PACK32; // A8B8G8R8

			if (pf.dwRBitMask == 0x000000ff && pf.dwGBitMask == 0x0000ff00 &&
				pf.dwBBitMask == 0x00ff0000 && pf.dwABitMask == 0x00000000)
				return Format::A8B8G8R8_UNORM_PACK32; // X8B8G8R8

			if (pf.dwRBitMask == 0x3ff00000 && pf.dwGBitMask == 0x000ffc00 &&
				pf.dwBBitMask == 0x000003ff && pf.dwABitMask == 0xc0000000)
				return Format::A2R10G10B10_UNORM_PACK32; // A2R10G10B10

			if (pf.dwRBitMask == 0x000003ff && pf.dwGBitMask == 0x000ffc00 &&
				pf.dwBBitMask == 0x3ff00000 && pf.dwABitMask == 0xc0000000)
				return Format::A2B10G10R10_UNORM_PACK32; // A2B10G10R10

			if (pf.dwRBitMask == 0x0000ffff && pf.dwGBitMask == 0xffff0000)
				return Format::R16G16_UNORM;  // G16R16

			break;

		case 24:
			if (pf.dwRBitMask == 0x00ff0000 && pf.dwGBitMask == 0x0000ff00 &&
				pf.dwBBitMask == 0x000000ff)
				return Format::B8G8R8_UNORM; // R8G8B8

			break;

		case 16:
			if (pf.dwRBitMask == 0x7c00 && pf.dwGBitMask == 0x03e0 &&
				pf.dwBBitMask == 0x001f && pf.dwABitMask == 0x8000)
				return Format::A1R5G5B5_UNORM_PACK16; // A1R5G5B5

			if (pf.dwRBitMask == 0x7c00 && pf.dwGBitMask == 0x03e0 &&
				pf.dwBBitMask == 0x001f && pf.dwABitMask == 0x0000)
				return Format::A1R5G5B5_UNORM_PACK16; // X1R5G5B5

			if (pf.dwRBitMask == 0x0f00 && pf.dwGBitMask == 0x00f0 &&
				pf.dwBBitMask == 0x000f && pf.dwABitMask == 0xf000)
				return Format::A4R4G4B4_UNORM_PACK16; // A4R4G4B4

			if (pf.dwRBitMask == 0x0f00 && pf.dwGBitMask == 0x00f0 &&
				pf.dwBBitMask == 0x000f && pf.dwABitMask == 0x0000)
				return Format::A4R4G4B4_UNORM_PACK16; // X4R4G4B4

			if (pf.dwRBitMask == 0xf800 && pf.dwGBitMask == 0x07e0 &&
				pf.dwBBitMask == 0x001f)
				return Format::R5G6B5_UNORM_PACK16; // R5G6B5

			if (pf.dwRBitMask == 0xe0 && pf.dwGBitMask == 0x1c &&
				pf.dwBBitMask == 0x03 && pf.dwABitMask == 0xff00)
				return Format::Invalid;// A8R3G3B2

			break;
		}
	}

	// --- ALPHA only ---
	if (pf.dwFlags & DDPF_ALPHA) {
		if (pf.dwRGBBitCount == 8 && pf.dwABitMask == 0xff)
			return Format::A8_UNORM; // A8
	}

	// --- LUMINANCE formats ---
	if (pf.dwFlags & DDPF_LUMINANCE) {
		if (pf.dwRGBBitCount == 16) {
			if (pf.dwRBitMask == 0xffff)
				return Format::R16_UNORM; // L16
			if (pf.dwRBitMask == 0xff && pf.dwABitMask == 0xff00)
				return Format::Invalid; // A8L8
		}
		else if (pf.dwRGBBitCount == 8) {
			if (pf.dwRBitMask == 0xff)
				return Format::R8_UNORM;// L8
			if (pf.dwRBitMask == 0x0f && pf.dwABitMask == 0xf0)
				return Format::Invalid; // A4L4
		}
	}

	return Format::Invalid;
}

bool Grindstone::Formats::DDS::TryParseDds(const char* debugName, Grindstone::Containers::BufferSpan bufferView, DdsParseOutput& output) {
	char* fileContents = reinterpret_cast<char*>(&bufferView.GetBegin());
	if (strncmp(fileContents, "DDS ", 4) != 0) {
		GPRINT_WARN_V(LogSource::EngineCore, "Invalid texture file: {}", debugName);
		return false;
	}

	DDSHeader header;
	std::memcpy(&header, fileContents + 4, sizeof(header));

	bool useDxgi = false;
	Grindstone::GraphicsAPI::Format format = GraphicsAPI::Format::Invalid;

	bool isFourCC = (header.ddspf.dwFlags & DDPF_FOURCC) != 0;
	bool isPixelFormat = (header.ddspf.dwFlags & (DDPF_ALPHAPIXELS | DDPF_ALPHA | DDPF_FOURCC | DDPF_RGB | DDPF_YUV | DDPF_LUMINANCE)) != 0;
	bool hasAlpha = (header.ddspf.dwFlags & DDPF_ALPHAPIXELS) != 0;

	bool hasCubemap = (header.dwCaps2 & DDSCAPS2_CUBEMAP) != 0;

	uint32_t width = header.dwWidth;
	uint32_t height = header.dwHeight;
	uint32_t depth = (header.dwDepth == 0) ? 1u : header.dwDepth;
	uint32_t mipCount = (header.dwMipMapCount > 1) ? header.dwMipMapCount : 1u;

	if (isFourCC) {
		if (header.ddspf.dwFourCC == FOURCC_DXGI) {
			useDxgi = true;
		}
		else {
			format = MapFourCC(header.ddspf.dwFourCC, hasAlpha);

			if (format == GraphicsAPI::Format::Invalid) {
				GPRINT_ERROR_V(LogSource::EngineCore, "Invalid FourCC in texture with name {}.", debugName);
				return false;
			}
		}
	}
	else if (isPixelFormat) {
		format = DetectFormatFromPixelFormat(header.ddspf);
		if (format == GraphicsAPI::Format::Invalid) {
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid FourCC in texture with name {}.", debugName);
			return false;
		}
	}
	else {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid pixel format in texture with name {}.", debugName);
		return false;
	}

	uint32_t arraySize = 1u;

	Grindstone::GraphicsAPI::ImageDimension imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
	char* imgPtr = fileContents + 4 + sizeof(DDSHeader);
	if (useDxgi) {
		DDSHeaderExtended extendedHeader;
		std::memcpy(&extendedHeader, imgPtr, sizeof(DDSHeaderExtended));
		format = MapDXGIToFormat(extendedHeader.dxgiFormat);

		if (format == Grindstone::GraphicsAPI::Format::Invalid) {
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid extended DXGI format in texture with name {}.", debugName);
			return false;
		}

		arraySize = std::max<uint32_t>(1u, extendedHeader.arraySize);

		switch (extendedHeader.resourceDimension) {
		default:
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid dimensions in texture with name({}).", debugName);
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Invalid;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_UNKNOWN:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Invalid;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_BUFFER:
			GPRINT_ERROR_V(LogSource::EngineCore, "Invalid dimensions \"Buffer\" in texture with name({}).", debugName);
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Invalid;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE1D:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension1D;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE2D:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
			break;
		case D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE3D:
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension3D;
			break;
		}

		if ((extendedHeader.miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE) != 0u) {
			hasCubemap = true;
		}

		imgPtr += sizeof(DDSHeaderExtended);
	}
	else {
		arraySize = hasCubemap ? (arraySize * 6u) : arraySize;
		imageDimensions = (depth > 1)
			? Grindstone::GraphicsAPI::ImageDimension::Dimension3D
			: Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
	}

	if (imageDimensions == Grindstone::GraphicsAPI::ImageDimension::Invalid) {
		if (height == 1) {
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension1D;
		}
		else if (depth == 1) {
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D;
		}
		else {
			imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension3D;
		}
	}

	auto bufferBegin = reinterpret_cast<char*>(&bufferView.GetBegin());
	size_t offset = imgPtr - bufferBegin;
	Grindstone::Containers::BufferSpan bufferSpan = bufferView.GetSubspan(static_cast<size_t>(imgPtr - bufferBegin), bufferView.GetSize() - offset);
	output = DdsParseOutput{
		.isCubemap = hasCubemap,
		.width = header.dwWidth,
		.height = header.dwHeight,
		.depth = header.dwDepth > 0 ? header.dwDepth : 1,
		.mipLevels = header.dwMipMapCount > 0 ? static_cast<uint32_t>(header.dwMipMapCount) : 1,
		.arraySize = arraySize,
		.dimensions = imageDimensions,
		.format = format,
		.data = bufferSpan
	};

	return true;
}

static bool ComputeMipSize(uint32_t width, uint32_t height, Grindstone::GraphicsAPI::Format format, size_t& outSize) {
	if (Grindstone::GraphicsAPI::IsFormatCompressed(format)) {
		uint32_t blockWidth = std::max<uint32_t>(1u, (width + 3) / 4) * 4u;
		uint32_t blockHeight = std::max<uint32_t>(1u, (height + 3) / 4) * 4u;
		size_t bytesPerBlock = Grindstone::GraphicsAPI::GetFormatBytesPerPixel(format);
		if (bytesPerBlock == 0) return false;
		outSize = static_cast<size_t>(blockWidth) * static_cast<size_t>(blockHeight) * bytesPerBlock;
		return true;
	}

	size_t bpp = Grindstone::GraphicsAPI::GetFormatBytesPerPixel(format);
	if (bpp == 0) {
		return false;
	}

	outSize = static_cast<size_t>(width) * static_cast<size_t>(height) * bpp;
	return true;
}

uint64_t Grindstone::Formats::DDS::GetRequiredBytes(const DdsParseOutput& ddsData) {
	size_t requiredBytes = 0;
	for (uint32_t layer = 0; layer < ddsData.arraySize; ++layer) {
		uint32_t faces = ddsData.isCubemap ? 6u : 1u;
		for (uint32_t face = 0; face < faces; ++face) {
			uint32_t tw = ddsData.width;
			uint32_t th = ddsData.height;
			uint32_t depthMip = ddsData.depth;
			for (uint32_t m = 0; m < ddsData.mipLevels; ++m) {
				size_t mipBytes = 0;
				if (!ComputeMipSize(tw, th, ddsData.format, mipBytes)) {
					return false;
				}
				if (ddsData.dimensions == Grindstone::GraphicsAPI::ImageDimension::Dimension3D) {
					mipBytes *= std::max<uint32_t>(1u, depthMip);
				}
				requiredBytes += mipBytes;
				tw = std::max<uint32_t>(1u, tw >> 1);
				th = std::max<uint32_t>(1u, th >> 1);
				if (ddsData.dimensions == Grindstone::GraphicsAPI::ImageDimension::Dimension3D) {
					depthMip = std::max<uint32_t>(1u, depthMip >> 1);
				}
			}
		}
	}

	return requiredBytes;
}
