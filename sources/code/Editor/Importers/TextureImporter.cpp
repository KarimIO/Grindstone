#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_dxt.h>
#include <stb_image_resize.h>

#include "Common/ResourcePipeline/MetaFile.hpp"
#include "Common/Formats/Dds.hpp"
#include "Editor/EditorManager.hpp"
#include "TextureImporter.hpp"
using namespace Grindstone::Importers;

const uint32_t blockWidth = 4;

void TextureImporter::ExtractBlock(
	const uint8_t* inPtr,
	const uint32_t levelWidth,
	uint8_t* colorBlock
) {
	if (texChannels == 3) {
		for (uint32_t dstRow = 0; dstRow < blockWidth; dstRow++) {
			for (uint32_t dstCol = 0; dstCol < blockWidth; dstCol++) {
				memcpy(colorBlock + (dstRow * blockWidth + dstCol) * 4, inPtr + (dstRow * levelWidth + dstCol) * texChannels, texChannels);
				colorBlock[(dstRow * blockWidth + dstCol) * 4 + 3] = 255;
			}
		}
		return;
	}

	for (uint32_t dstRow = 0; dstRow < blockWidth; dstRow++) {
		for (uint32_t dstCol = 0; dstCol < blockWidth; dstCol++) {
			memcpy(colorBlock + (dstRow * blockWidth + dstCol) * texChannels, inPtr + (dstRow * levelWidth + dstCol) * texChannels, texChannels);
		}
	}
}


void TextureImporter::Import(std::filesystem::path& path) {
	this->path = path;
	int width, height, channels;
	sourcePixels = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
	if (!sourcePixels) {
		throw std::runtime_error("Unable to load texture!");
	}

	texWidth = static_cast<uint32_t>(width);
	texHeight = static_cast<uint32_t>(height);
	texChannels = static_cast<uint32_t>(channels);
	targetTexChannels = texChannels;

	metaFile = new MetaFile(path);

	if (texChannels == 1) {
		compression = Compression::BC4;
	}
	else if (texChannels == 3) {
		compression = Compression::BC1;
	}
	else if (texChannels == 4) {
		compression = Compression::BC3;
	}
	else {
		compression = Compression::BC3;
	}

	switch (compression) {
	case Compression::BC1:
	case Compression::BC3:
		ConvertBC123();
		break;
	case Compression::BC4:
		ConvertBC4();
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

uint8_t TextureImporter::CombinePixels(uint8_t* pixelSrc, uint64_t index, uint64_t pitch) {
	uint32_t pixelA = static_cast<uint8_t>(*(pixelSrc + index));
	uint32_t pixelB = static_cast<uint8_t>(*(pixelSrc + (index + texChannels)));
	uint32_t pixelC = static_cast<uint8_t>(*(pixelSrc + (index + pitch)));
	uint32_t pixelD = static_cast<uint8_t>(*(pixelSrc + (index + pitch + texChannels)));
	uint32_t average = (pixelA + pixelB + pixelC + pixelD) / 4;
	return static_cast<uint8_t>(average);
}

uint8_t *TextureImporter::CreateMip(uint8_t *pixel, uint32_t width, uint32_t height) {
	uint64_t pitch = static_cast<uint64_t>(width * 2 * texChannels);
	uint64_t size = static_cast<uint64_t>(pitch * height * 2);
	uint8_t *mip = new uint8_t[size];
	uint64_t dst = 0;
	for (uint64_t srcRow = 0; srcRow < height; srcRow++) {
		for (uint64_t srcCol = 0; srcCol < width; srcCol++) {
			uint64_t index = (srcRow * pitch) + (srcCol * texChannels);
			uint8_t* src = pixel + index;
			for (uint64_t channelIndex = 0; channelIndex < texChannels; ++channelIndex) {
				mip[dst++] = CombinePixels(src, index + channelIndex, pitch);
			}
		}
	}

	return mip;
}

void TextureImporter::ConvertBC123() {
	bool isSixSidedCubemap = false;
	// if (isSixSidedCubemap)
	// 	size *= 6;

	bool shouldGenerateMips = true;
	int mipMapCount = shouldGenerateMips
		? CalculateMipMapLevelCount(texWidth, texHeight)
		: 1;

	int outputContentSize = 0;
	uint32_t blockSize = (compression == Compression::BC1) ? 8 : 16;
	int mipWidth = texWidth;
	int mipHeight = texHeight;
	for (int i = 0; i < mipMapCount; i++) {
		int mipsize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;
		outputContentSize += mipsize;

		mipWidth /= 2;
		mipHeight /= 2;
	}

	int faceCount = isSixSidedCubemap ? 6 : 1;
	outputContentSize *= faceCount;

	uint8_t* outData = new uint8_t[outputContentSize];
	int minMipLevel = std::max(mipMapCount, 1);
	for (int faceIterator = 0; faceIterator < faceCount; faceIterator++) {
		GenerateFaceBC123(minMipLevel, faceIterator, outData);
	}

	OutputDds(outData, outputContentSize);
}


void TextureImporter::ConvertBC4() {
	bool isSixSidedCubemap = false;
	// if (isSixSidedCubemap)
	// 	size *= 6;

	bool shouldGenerateMips = true;
	int mipMapCount = shouldGenerateMips ?
		CalculateMipMapLevelCount(texWidth, texHeight)
		: 1;

	int outputContentSize = 0;
	uint32_t blockSize = 8;
	int mipWidth = texWidth;
	int mipHeight = texHeight;
	for (int i = 0; i < mipMapCount; i++) {
		int mipsize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;
		outputContentSize += mipsize;

		mipWidth /= 2;
		mipHeight /= 2;
	}

	int faceCount = isSixSidedCubemap ? 6 : 1;
	outputContentSize *= faceCount;

	uint8_t* outData = new uint8_t[outputContentSize];
	int minMipLevel = std::max(mipMapCount, 1);
	for (int faceIterator = 0; faceIterator < faceCount; faceIterator++) {
		GenerateFaceBC4(minMipLevel, faceIterator, outData);
	}

	OutputDds(outData, outputContentSize);
}

void TextureImporter::GenerateMipList(uint32_t minMipLevel, std::vector<uint8_t*>& uncompressedMips) {
	uint32_t levelWidth = texWidth;
	uint32_t levelHeight = texHeight;

	uncompressedMips.resize(minMipLevel);
	uncompressedMips[0] = sourcePixels;
#ifdef SHOULD_EXPORT_NORMAL_IMAGES
	std::filesystem::path filename = std::filesystem::path("D:/MipGen") / path.filename();
	stbi_write_tga(filename.string().c_str(), levelWidth, levelHeight, texChannels, uncompressedMips[0]);
#endif
	for (uint32_t mipLevelIterator = 1; mipLevelIterator < minMipLevel; mipLevelIterator++) {
		levelWidth /= 2;
		levelHeight /= 2;

		uncompressedMips[mipLevelIterator] = CreateMip(uncompressedMips[mipLevelIterator - 1], levelWidth, levelHeight);

#ifdef SHOULD_EXPORT_NORMAL_IMAGES
		std::string extension = "_mip" + std::to_string(mipLevelIterator) + ".tga";
		std::string fileName = path.filename().replace_extension("").string() + extension;
		std::filesystem::path filename = std::filesystem::path("D:/MipGen") / fileName;
		stbi_write_tga(filename.string().c_str(), levelWidth, levelHeight, texChannels, uncompressedMips[mipLevelIterator]);
#endif
	}
}

void TextureImporter::GenerateFaceBC123(uint32_t minMipLevel, uint32_t faceIterator, uint8_t* outData) {
	uint32_t blockSize = (compression == Compression::BC1) ? 8 : 16;
	uint32_t dstOffset = 0;
	uint8_t block[64];

	std::vector<uint8_t*> uncompressedMips;
	GenerateMipList(minMipLevel, uncompressedMips);

	uint32_t levelWidth = texWidth;
	uint32_t levelHeight = texHeight;

	bool hasAlpha = texChannels == 4;
	for (uint32_t mipLevelIterator = 0; mipLevelIterator < minMipLevel; mipLevelIterator++) {
		uint8_t* mipSource = uncompressedMips[mipLevelIterator];

		for (uint32_t mipRow = 0; mipRow < levelHeight; mipRow += blockWidth) {
			for (uint32_t mipCol = 0; mipCol < levelWidth; mipCol += blockWidth) {
				uint8_t* ptr = mipSource + ((mipRow * levelWidth + mipCol) * texChannels);
				ExtractBlock(ptr, levelWidth, block);
				stb_compress_dxt_block(outData + dstOffset, block, hasAlpha, STB_DXT_NORMAL);
				dstOffset += blockSize;
			}
		}

		levelWidth /= 2;
		levelHeight /= 2;
	}
}

void TextureImporter::GenerateFaceBC4(uint32_t minMipLevel, uint32_t faceIterator, uint8_t* outData) {
	uint32_t dstOffset = 0;
	uint8_t block[64];

	std::vector<uint8_t*> uncompressedMips;
	GenerateMipList(minMipLevel, uncompressedMips);

	uint32_t levelWidth = texWidth;
	uint32_t levelHeight = texHeight;

	for (uint32_t mipLevelIterator = 0; mipLevelIterator < minMipLevel; mipLevelIterator++) {
		uint8_t* mipSource = uncompressedMips[mipLevelIterator];

		for (uint32_t mipRow = 0; mipRow < levelHeight; mipRow += blockWidth) {
			uint8_t* ptr = mipSource + (mipRow * levelWidth * texChannels);
			for (uint32_t mipCol = 0; mipCol < levelWidth; mipCol += blockWidth) {
				ExtractBlock(ptr, levelWidth, block);
				stb_compress_bc4_block(&outData[dstOffset], block);
				ptr += blockWidth * texChannels;
				dstOffset += 8;
			}
		}

		levelWidth /= 2;
		levelHeight /= 2;
	}
}

void TextureImporter::OutputDds(uint8_t* outData, uint32_t contentSize) {
	bool shouldGenerateMips = true;

	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_REQUIRED | DDSD_MIPMAPCOUNT;
	outHeader.dwHeight = texHeight;
	outHeader.dwWidth = texWidth;
	outHeader.dwDepth = 0;
	outHeader.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
	outHeader.dwMipMapCount = 0;

	if (shouldGenerateMips) {
		outHeader.dwCaps |= DDSCAPS_MIPMAP;
		outHeader.dwMipMapCount = DWORD(CalculateMipMapLevelCount(texWidth, texHeight));
	}

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
	case Compression::BC4:
		outHeader.dwPitchOrLinearSize = texWidth * texHeight / 2;
		outHeader.ddspf.dwFourCC = FOURCC_BC4;
		break;
	}
	char mark[] = { 'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E' };
	std::memcpy(&outHeader.dwReserved1, mark, sizeof(mark));

	std::string subassetName = "texture";
	uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Texture);
	std::filesystem::path outputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
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

uint32_t TextureImporter::CalculateMipMapLevelCount(uint32_t width, uint32_t height) {
	uint32_t size = std::min(width, height);
	return static_cast<uint32_t>(std::log2(size) - 1);
}

void Grindstone::Importers::ImportTexture(std::filesystem::path& inputPath) {
	TextureImporter TextureImporter;
	TextureImporter.Import(inputPath);
}
