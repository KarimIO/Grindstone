#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION
#include "ImageConverter.hpp"

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
			mip[++dst]	= pixel[src];
			mip[++dst]  = pixel[src + 1];
			mip[++dst]  = pixel[src + 2];
			mip[++dst]  = pixel[src + 3];
		}
	}

	return mip;
}

void ConvertBC123(unsigned char *pixels, int width, int height, Compression compression, std::string path) {
	// No Alpha
	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE | DDSD_MIPMAPCOUNT;
	outHeader.dwHeight = height;
	outHeader.dwWidth = width;
	outHeader.dwDepth = 0;
	outHeader.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	outHeader.dwMipMapCount = std::log2(width)-2;
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
	size = useMip ? 1.5 * size : size;
	unsigned char *outData = new unsigned char[size];
	int offset = 0;
	unsigned char block[64];
	unsigned char *mip = pixels;
	
	for (int k = 0; k < outHeader.dwMipMapCount; k++) {
		for (int j = 0; j < height; j+=4) {
			unsigned char *ptr = mip + j * width * 4;
			for (int i = 0; i < width; i+=4) {
				ExtractBlock(ptr, width, block);
				stb_compress_dxt_block(&outData[offset], block, false, STB_DXT_DITHER);
				ptr += 4 * 4;
				offset += 8;
			}
		}
		width /= 2;
		height /= 2;
		
		unsigned char *temp_mip = mip;

		if (k-1 != outHeader.dwMipMapCount)
			mip = CreateMip(temp_mip, width, height);

		if (k != 0)
			delete[] temp_mip;
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

bool ConvertTexture(std::string input, std::string output, Compression compression) {
    std::string path = input;
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, 4);
    if (!pixels) {
        printf("Texture failed to load!: %s \nPress enter to exit. ", path.c_str());
		std::cin.get();
        return false;
    }

    switch(compression) {
        default:
            ConvertBC123(pixels, texWidth, texHeight, C_BC1, output);
            break;
    }

    delete[] pixels;

    return true;
}