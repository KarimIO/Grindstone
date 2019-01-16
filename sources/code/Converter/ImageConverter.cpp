#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION
#include "ImageConverter.hpp"

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

#define DDSCAPS2_CUBEMAP			0x200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x1000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x2000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x4000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x8000
#define DDSCAPS2_CUBEMAP_VOLUME		0x200000

#define DDS_CUBEMAP_ALLFACES		DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ;

void ExtractBlock(const unsigned char* inPtr, unsigned int width, unsigned char* colorBlock) {
    for (int j = 0; j < 4; j++) {
        memcpy(&colorBlock[j * 4 * 4], inPtr, 4 * 4);
        inPtr += width * 4;
    }
}

unsigned char *CreateMip(unsigned char *pixel, int width, int height) {
	int size = width * height;
	unsigned char *mip = new unsigned char[size * 4];
	int dst = -1;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int src = (i * width * 4 + j * 2) * 4;
			mip[++dst]	= pixel[src] / 4;
			mip[dst]	+= pixel[src + 4] / 4;
			mip[dst]	+= pixel[src + 8] / 4;
			mip[dst]	+= pixel[src + 12] / 4;

			mip[++dst]  = pixel[src + 1] / 4;
			mip[dst]  += pixel[src + 5] / 4;
			mip[dst]  += pixel[src + 9] / 4;
			mip[dst]  += pixel[src + 13] / 4;

			mip[++dst]  = pixel[src + 2] / 4;
			mip[dst]  += pixel[src + 6] / 4;
			mip[dst]  += pixel[src + 10] / 4;
			mip[dst]  += pixel[src + 14] / 4;

			mip[++dst]  = pixel[src + 3] / 4;
			mip[dst]  += pixel[src + 7] / 4;
			mip[dst]  += pixel[src + 11] / 4;
			mip[dst]  += pixel[src + 15] / 4;
		}
	}

	return mip;
}

void ConvertBC123(unsigned char **pixels, bool is_cubemap, int width, int height, Compression compression, std::string path) {
	// No Alpha
	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE | DDSD_MIPMAPCOUNT;
	outHeader.dwHeight = height;
	outHeader.dwWidth = width;
	outHeader.dwDepth = 0;
	outHeader.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	outHeader.dwMipMapCount = std::log2(width)+1;
	if (is_cubemap)
		outHeader.dwCaps2 = DDS_CUBEMAP_ALLFACES;

	bool alpha = false;
	switch (compression) {
	default:
	case C_BC1: {
		outHeader.dwPitchOrLinearSize = width * height / 2;
		outHeader.ddspf.dwFourCC = FOURCC_DXT1;
		break;
	}
	case C_BC2:
		outHeader.dwPitchOrLinearSize = width * height;
		outHeader.ddspf.dwFourCC = FOURCC_DXT3;
		break;
	case C_BC3:
		outHeader.dwPitchOrLinearSize = width * height;
		outHeader.ddspf.dwFourCC = FOURCC_DXT5;
		break;
	}
	char mark[] = {'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E'};
	std::memcpy(&outHeader.dwReserved1, mark, sizeof(mark));

	bool useMip = outHeader.dwMipMapCount > 1;
	int size = outHeader.dwPitchOrLinearSize;
	int mipsize = size;
	unsigned int blockSize = (outHeader.ddspf.dwFourCC == FOURCC_DXT1) ? 8 : 16;
	for (int i = 1; i < outHeader.dwMipMapCount; i++) {
		mipsize = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		size += mipsize;
	}

	if (is_cubemap)
		size *= 6;
	
	unsigned char *outData = new unsigned char[size];
	int offset = 0;
	unsigned char block[64];

	int minlev = outHeader.dwMipMapCount - 2;
	int num_faces = is_cubemap ? 6 : 1;
	for (int l = 0; l < num_faces; l++) {
		unsigned char *mip = pixels[l];
		width = outHeader.dwWidth;
		height = outHeader.dwHeight;
		for (int k = 0; k < minlev; k++) {
			for (int j = 0; j < height; j+=4) {
				unsigned char *ptr = mip + j * width * 4;
				for (int i = 0; i < width; i+=4) {
					ExtractBlock(ptr, width, block);
					stb_compress_dxt_block(&outData[offset], block, false, STB_DXT_NORMAL);
					ptr += 4 * 4;
					offset += 8;
				}
			}
			width /= 2;
			height /= 2;
			
			unsigned char *temp_mip = mip;

			if (k-1 != minlev)
				mip = CreateMip(temp_mip, width, height);

			if (k != 0)
				delete[] temp_mip;
		}

		memcpy(&outData[offset], &outData[offset], 8); // 2x2
		offset += 8;
		memcpy(&outData[offset], &outData[offset], 8); // 1x1
		offset += 8;
	}

	std::ofstream out(path, std::ios::binary);
	if (out.fail()) {
		std::cerr << "Failed to output to " + path + ".\n";
		return;
	}
	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char *)&filecode, sizeof(char) * 4);
	out.write((const char *)&outHeader, sizeof(outHeader));
	out.write((const char *)outData, size);
	out.close();

	delete[] outData;
}

bool ConvertTexture(std::string input, bool is_cubemap, std::string output, Compression compression) {
    std::string *path;
	if (is_cubemap) {
		path = new std::string[6];
		int p = input.find_last_of('.');
		std::string pre = input.substr(0, p);
		std::string ext = input.substr(p);
		path[0] = pre + "_ft" + ext;
		path[1] = pre + "_bk" + ext;
		path[2] = pre + "_up" + ext;
		path[3] = pre + "_dn" + ext;
		path[4] = pre + "_rt" + ext;
		path[5] = pre + "_lf" + ext;
	}
	else {
		path = new std::string[1];
		path[0] = input;
	}
    int texWidth, texHeight, texChannels;
	int num_maps = is_cubemap ? 6 : 1;
	stbi_uc **pixels = new stbi_uc*[num_maps];
    for (int i = 0; i < num_maps; i++) {
		pixels[i] = stbi_load(path[i].c_str(), &texWidth, &texHeight, &texChannels, 4);
		if (!pixels[i]) {
			printf("Texture failed to load!: %s", path[i].c_str());
			for (int j = 0; j < i; i++) {
				delete[] pixels[j];
			}
			delete[] pixels;
			return false;
		}
	}

	if (compression == C_DETECT)
		if (texChannels == 4) {
			compression = C_BC3;
		}
		else {
			compression = C_BC1;
		}

    switch(compression) {
        default:
            ConvertBC123(pixels, is_cubemap, texWidth, texHeight, compression, output);
            break;
    }

    delete[] pixels;

    return true;
}