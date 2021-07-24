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

constexpr DWORD MakeFourCC(char c0, char c1, char c2, char c3) {
	 return ((DWORD)(char)(c0) | ((DWORD)(char)(c1) << 8) | \
		((DWORD)(char)(c2) << 16) | ((DWORD)(char)(c3) << 24));
}

constexpr DWORD MakeFourCCStr(const char* str) {
	return MakeFourCC(str[0], str[1], str[2], str[3]);
}

constexpr DWORD FOURCC_DXT1 = MakeFourCC('D', 'X', 'T', '1');
constexpr DWORD FOURCC_DXT3 = MakeFourCC('D', 'X', 'T', '3');
constexpr DWORD FOURCC_DXT5 = MakeFourCC('D', 'X', 'T', '5');
constexpr DWORD FOURCC_BC4 = MakeFourCC('A', 'T', 'I', '1');
constexpr DWORD FOURCC_BC5 = MakeFourCC('A', 'T', 'I', '2');

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
