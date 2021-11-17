#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_dxt.h>
#include <stb_image_resize.h>

#include "Common/ResourcePipeline/MetaFile.hpp"
#include "Common/Formats/Dds.hpp"
#include "TextureImporter.hpp"
using namespace Grindstone::Importers;

const unsigned int blockWidth = 4;

void TextureImporter::ExtractBlock(
	const unsigned char* inPtr,
	unsigned char* colorBlock
) {
	for (int j = 0; j < 4; j++) {
		memcpy(&colorBlock[j * texChannels * blockWidth], inPtr, texChannels * blockWidth);
		inPtr += texWidth * texChannels;
	}
}


void TextureImporter::Import(std::filesystem::path& path) {
	this->path = path;
	sourcePixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, 4);
	if (!sourcePixels) {
		throw std::runtime_error("Unable to load texture!");
	}

	metaFile = new MetaFile(path);

	Compression compression;
	if (texChannels == 4) {
		compression = Compression::BC3;
	}
	else {
		compression = Compression::BC1;
	}

	switch (compression) {
	case Compression::BC1:
	case Compression::BC3:
		ConvertBC123();
		break;
	default:
		delete sourcePixels;
		throw "Invalid compression type!";
	}

	delete sourcePixels;
}

Grindstone::Uuid TextureImporter::GetUuid() {
	return uuid;
}

unsigned char TextureImporter::CombinePixels(unsigned char* pixelSrc) {
	return (*pixelSrc + *(pixelSrc + 4) + *(pixelSrc + 8) + *(pixelSrc + 12)) / 4;
}

unsigned char *TextureImporter::CreateMip(unsigned char *pixel, int width, int height) {
	int size = width * height * texChannels;
	unsigned char *mip = new unsigned char[size];
	int dst = -1;
	for (int srcRow = 0; srcRow < height; srcRow++) {
		for (int srcCol = 0; srcCol < width; srcCol++) {
			int src = (srcRow * width + srcCol) * texChannels;
			mip[++dst] = CombinePixels(pixel + src);
			mip[++dst] = CombinePixels(pixel + src + 1);
			mip[++dst] = CombinePixels(pixel + src + 2);
			mip[++dst] = CombinePixels(pixel + src + 3);
		}
	}

	return mip;
}

void TextureImporter::ConvertBC123() {
	bool isSixSidedCubemap = false;
	// if (isSixSidedCubemap)
	// 	size *= 6;

	bool shouldGenerateMips = false;
	int mipMapCount = shouldGenerateMips ?
		CalculateMipMapLevelCount(texWidth, texHeight)
		: 1;

	int outputContentSize = 0;
	unsigned int blockSize = (compression == Compression::BC1) ? 8 : 16;
	int mipWidth = texWidth;
	int mipHeight = texHeight;
	for (int i = 0; i < mipMapCount; i++) {
		int mipsize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;
		outputContentSize += mipsize;

		mipWidth /= 2;
		mipHeight /= 2;
	}

	unsigned char* outData = new unsigned char[outputContentSize];
	int dstOffset = 0;
	unsigned char block[64];

	int minMipLevel = std::max(mipMapCount - 2, 1);
	int faceCount = isSixSidedCubemap ? 6 : 1;
	int levelWidth = texWidth;
	int levelHeight = texHeight;
	for (int faceIterator = 0; faceIterator < faceCount; faceIterator++) {
		unsigned char* mipSourcePtr = sourcePixels;
		for (int mipLevelIterator = 0; mipLevelIterator < minMipLevel; mipLevelIterator++) {
			if (!shouldGenerateMips) {
				mipSourcePtr = &sourcePixels[mipLevelIterator];
			}

			for (int mipRow = 0; mipRow < levelHeight; mipRow += blockWidth) {
				unsigned char *ptr = mipSourcePtr + (mipRow * levelWidth * texChannels);
				for (int mipCol = 0; mipCol < levelWidth; mipCol += blockWidth) {
					ExtractBlock(ptr, block);
					stb_compress_dxt_block(&outData[dstOffset], block, false, STB_DXT_NORMAL);
					ptr += blockWidth * texChannels;
					dstOffset += 8;
				}
			}

			levelWidth /= 2;
			levelHeight /= 2;

			if (shouldGenerateMips) {
				unsigned char *temp_mip = mipSourcePtr;

				if (mipLevelIterator - 1 != minMipLevel) {
					mipSourcePtr = CreateMip(temp_mip, levelWidth, levelHeight);
				}

				if (mipLevelIterator != 0) {
					delete[] temp_mip;
				}
			}

			if (levelWidth < 4) {
				break;
			}
		}

		/*memcpy(&outData[offset], &outData[offset], 8); // 2x2
		offset += 8;
		memcpy(&outData[offset], &outData[offset], 8); // 1x1
		offset += 8;*/
	}

	OutputDds(outData, outputContentSize);
}

void TextureImporter::OutputDds(unsigned char* outData, int contentSize) {
	bool shouldGenerateMips = false;

	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_REQUIRED | DDSD_MIPMAPCOUNT;
	outHeader.dwHeight = texHeight;
	outHeader.dwWidth = texWidth;
	outHeader.dwDepth = 0;
	outHeader.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	outHeader.dwMipMapCount = shouldGenerateMips
		? DWORD(CalculateMipMapLevelCount(texWidth, texHeight))
		: 0;
	// if (isSixSidedCubemap)
	// 	outHeader.dwCaps2 = DDS_CUBEMAP_ALLFACES;

	switch (compression) {
	default:
	case Compression::BC1:
		outHeader.dwPitchOrLinearSize = texWidth * texHeight / 2;
		outHeader.ddspf.dwFourCC = FOURCC_DXT1;
		break;
	case Compression::BC3:
		outHeader.dwPitchOrLinearSize = texWidth * texHeight;
		outHeader.ddspf.dwFourCC = FOURCC_DXT5;
		break;
	}
	char mark[] = { 'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E' };
	std::memcpy(&outHeader.dwReserved1, mark, sizeof(mark));

	std::string basePath = "../compiledAssets/";
	std::string subassetName = "texture";
	uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName);
	std::string outputPath = basePath + uuid.ToString();
	std::ofstream out(outputPath, std::ios::binary);
	if (out.fail()) {
		throw std::runtime_error("Failed to output texture!");
	}
	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char*)&filecode, sizeof(char) * 4);
	out.write((const char*)&outHeader, sizeof(outHeader));
	out.write((const char*)outData, contentSize);
	out.close();

	metaFile->Save();

	delete[] outData;
}

int TextureImporter::CalculateMipMapLevelCount(int width, int height) {
	int size = std::min(width, height);
	return (int)std::log2(size) - 1;
}

void Grindstone::Importers::ImportTexture(std::filesystem::path& inputPath) {
	TextureImporter TextureImporter;
	TextureImporter.Import(inputPath);
}
