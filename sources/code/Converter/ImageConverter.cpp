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

void ConvertBC123(unsigned char *pixels, int width, int height, Compression compression, std::string path) {
	// No Alpha
	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_REQUIRED;
	outHeader.dwHeight = height;
	outHeader.dwWidth = width;
	outHeader.dwDepth = 0;
	outHeader.dwCaps = DDSCAPS_TEXTURE;
	outHeader.dwMipMapCount = 0;
	bool alpha = false;
	switch (compression) {
	default:
	case C_BC1: {
		outHeader.dwPitchOrLinearSize = width * height;
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

	int wq = width / 4;
	int hq = height / 4;
	unsigned char *outData = new unsigned char[outHeader.dwPitchOrLinearSize];
	unsigned char block[64];
    
	for (int j = 0; j < hq; j++) {
		unsigned char *ptr = pixels + j * 4 * width * 4;
		for (int i = 0; i < wq; i++) {
			int pos = j * wq + i;
			ExtractBlock(ptr, width, block);
			stb_compress_dxt_block(outData + pos * 8, block, false, STB_DXT_DITHER);
			ptr += 4 * 4;
		}
	}

	std::ofstream out(path, std::ios::binary);
	if (out.fail()) {
		std::cerr << "Failed to output to " + path + ".\n";
		return;
	}
	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char *)&filecode, sizeof(char) * 4);
	out.write((const char *)&outHeader, sizeof(outHeader));
	out.write((const char *)outData, outHeader.dwPitchOrLinearSize);
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