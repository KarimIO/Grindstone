#pragma once

using DWORD = unsigned long;

struct DDS_PIXELFORMAT {
	DWORD dwSize = 32;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

constexpr DWORD DDPF_ALPHAPIXELS = 0x1;
constexpr DWORD DDPF_FOURCC = 0x4;
constexpr DWORD DDPF_RGB  = 0x40;

constexpr DWORD DDSD_CAPS = 0x1;
constexpr DWORD DDSD_HEIGHT = 0x2;
constexpr DWORD DDSD_WIDTH = 0x4;
constexpr DWORD DDSD_PITCH = 0x8;
constexpr DWORD DDSD_PIXELFORMAT = 0x1000;
constexpr DWORD DDSD_MIPMAPCOUNT = 0x20000;
constexpr DWORD DDSD_LINEARSIZE = 0x80000;
constexpr DWORD DDSD_DEPTH = 0x800000;
constexpr DWORD DDSD_REQUIRED =
	DDSD_CAPS |
	DDSD_HEIGHT |
	DDSD_WIDTH |
	DDSD_PIXELFORMAT |
	DDSD_LINEARSIZE;

struct DDSHeader {
	DWORD           dwSize = 124;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
};

enum class DxgiFormat : uint32_t {
	DXGI_FORMAT_UNKNOWN = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,
	DXGI_FORMAT_R32G32_TYPELESS = 15,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,
	DXGI_FORMAT_R11G11B10_FLOAT = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,
	DXGI_FORMAT_R16G16_TYPELESS = 33,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,
	DXGI_FORMAT_R32_TYPELESS = 39,
	DXGI_FORMAT_D32_FLOAT = 40,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,
	DXGI_FORMAT_R24G8_TYPELESS = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
	DXGI_FORMAT_R8G8_TYPELESS = 48,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,
	DXGI_FORMAT_R16_TYPELESS = 53,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_D16_UNORM = 55,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,
	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,
	DXGI_FORMAT_R1_UNORM = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	DXGI_FORMAT_BC1_TYPELESS = 70,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	DXGI_FORMAT_BC2_TYPELESS = 73,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	DXGI_FORMAT_BC3_TYPELESS = 76,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	DXGI_FORMAT_BC4_TYPELESS = 79,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,
	DXGI_FORMAT_BC5_TYPELESS = 82,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,
	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	DXGI_FORMAT_BC6H_TYPELESS = 94,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,
	DXGI_FORMAT_BC7_TYPELESS = 97,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	DXGI_FORMAT_AYUV = 100,
	DXGI_FORMAT_Y410 = 101,
	DXGI_FORMAT_Y416 = 102,
	DXGI_FORMAT_NV12 = 103,
	DXGI_FORMAT_P010 = 104,
	DXGI_FORMAT_P016 = 105,
	DXGI_FORMAT_420_OPAQUE = 106,
	DXGI_FORMAT_YUY2 = 107,
	DXGI_FORMAT_Y210 = 108,
	DXGI_FORMAT_Y216 = 109,
	DXGI_FORMAT_NV11 = 110,
	DXGI_FORMAT_AI44 = 111,
	DXGI_FORMAT_IA44 = 112,
	DXGI_FORMAT_P8 = 113,
	DXGI_FORMAT_A8P8 = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM = 115,
	DXGI_FORMAT_P208 = 130,
	DXGI_FORMAT_V208 = 131,
	DXGI_FORMAT_V408 = 132,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
	DXGI_FORMAT_FORCE_UINT = 0xffffffff
};

enum class D3d10ResourceDimension : uint32_t {
	D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
};

struct DDSHeaderExtended {
	DxgiFormat dxgiFormat = DxgiFormat::DXGI_FORMAT_UNKNOWN;
	D3d10ResourceDimension resourceDimension = D3d10ResourceDimension::D3D10_RESOURCE_DIMENSION_TEXTURE2D;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
};

constexpr DWORD MakeFourCC(char c0, char c1, char c2, char c3) {
	 return ((DWORD)(char)(c0) | ((DWORD)(char)(c1) << 8) | \
		((DWORD)(char)(c2) << 16) | ((DWORD)(char)(c3) << 24));
}

constexpr DWORD MakeFourCCStr(const char* str) {
	return MakeFourCC(str[0], str[1], str[2], str[3]);
}

constexpr DWORD FOURCC_R16 = 111;
constexpr DWORD FOURCC_RG16 = 112;
constexpr DWORD FOURCC_RGBA16 = 113;
constexpr DWORD FOURCC_R32 = 114;
constexpr DWORD FOURCC_RG32 = 115;
constexpr DWORD FOURCC_RGBA32 = 116;
constexpr DWORD FOURCC_DXT1 = MakeFourCC('D', 'X', 'T', '1');
constexpr DWORD FOURCC_DXT3 = MakeFourCC('D', 'X', 'T', '3');
constexpr DWORD FOURCC_DXT5 = MakeFourCC('D', 'X', 'T', '5');
constexpr DWORD FOURCC_BC4 = MakeFourCC('A', 'T', 'I', '1');
constexpr DWORD FOURCC_BC5 = MakeFourCC('A', 'T', 'I', '2');
constexpr DWORD FOURCC_DXGI = MakeFourCC('D', 'X', '1', '0');
constexpr DWORD FOURCC_RGB16 = MakeFourCC('H', 'D', 'R', 'I'); // This is made up, and not supported, but I decided to use it because I don't want an alpha channel

constexpr DWORD DXGI_BC6H = 95;

constexpr DWORD DDSCAPS_COMPLEX = 0x8;
constexpr DWORD DDSCAPS_MIPMAP = 0x400000;
constexpr DWORD DDSCAPS_TEXTURE = 0x1000;

constexpr DWORD DDSCAPS2_CUBEMAP = 0x200;
constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
constexpr DWORD DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
constexpr DWORD DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;
constexpr DWORD DDSCAPS2_CUBEMAP_VOLUME = 0x200000;

constexpr DWORD DDS_CUBEMAP_ALLFACES =
	DDSCAPS2_CUBEMAP |
	DDSCAPS2_CUBEMAP_POSITIVEX |
	DDSCAPS2_CUBEMAP_NEGATIVEX |
	DDSCAPS2_CUBEMAP_POSITIVEY |
	DDSCAPS2_CUBEMAP_NEGATIVEY |
	DDSCAPS2_CUBEMAP_POSITIVEZ |
	DDSCAPS2_CUBEMAP_NEGATIVEZ;
