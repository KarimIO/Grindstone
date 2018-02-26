#ifndef IMAGE_CONVERTER_HPP
#define IMAGE_CONVERTER_HPP

#include <string>

#include <stb/stb_image.h>
#include <stb/stb_dxt.h>

typedef uint32_t DWORD;

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

#define DDPF_ALPHAPIXELS 0x1
#define DDPF_FOURCC 0x4
#define DDPF_RGB 0x40

#define DDSD_CAPS			0x1
#define DDSD_HEIGHT			0x2
#define DDSD_WIDTH			0x4
#define DDSD_PITCH			0x8
#define DDSD_PIXELFORMAT	0x1000
#define DDSD_MIPMAPCOUNT	0x20000
#define DDSD_LINEARSIZE		0x80000
#define DDSD_DEPTH			0x800000
#define DDSD_REQUIRED		DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;

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

#define MAKEFOURCC(c0, c1, c2, c3)	((DWORD)(char)(c0) | ((DWORD)(char)(c1) << 8) | \
									((DWORD)(char)(c2) << 16) | ((DWORD)(char)(c3) << 24))
#define MAKEFOURCCS(str)			MAKEFOURCC(str[0], str[1], str[2], str[3])
#define FOURCC_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT5 MAKEFOURCC('D', 'X', 'T', '5')
#define FOURCC_BC4 MAKEFOURCC('A', 'T', 'I', '1')
#define FOURCC_BC5 MAKEFOURCC('A', 'T', 'I', '2')

#define DDSCAPS_COMPLEX		0x8
#define DDSCAPS_MIPMAP		0x400000
#define DDSCAPS_TEXTURE		0x1000

enum Compression {
    C_DETECT = 0,
	C_UNCOMPRESSED,
	C_BC1,
	C_BC2,
	C_BC3,
	C_BC4,
	C_BC5,
	C_BC6H,
	C_BC7
};

void ExtractBlock(const unsigned char* inPtr, unsigned int width, unsigned char* colorBlock);

void ConvertBC123(unsigned char *pixels, int width, int height, Compression compression, std::string path);

bool ConvertTexture(std::string input, std::string output, Compression compression = C_DETECT);

#endif