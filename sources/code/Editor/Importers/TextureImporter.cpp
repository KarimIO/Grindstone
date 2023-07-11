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

	sourceWidth = static_cast<uint32_t>(width);
	sourceHeight = static_cast<uint32_t>(height);
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
		throw std::runtime_error("Invalid compression type!");
	}

	isSixSidedCubemap = (width / 4 == height / 3);
	texWidth = isSixSidedCubemap ? sourceWidth / 4 : sourceWidth;
	texHeight = isSixSidedCubemap ? sourceHeight / 3 : sourceHeight;

	Convert();

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

void TextureImporter::Convert() {
	int mipMapCount = shouldGenerateMips
		? CalculateMipMapLevelCount(texWidth, texHeight)
		: 1;

	uint64_t outputFaceSize = 0;
	uint32_t blockSize = (compression == Compression::BC3) ? 16 : 8;
	int mipWidth = texWidth;
	int mipHeight = texHeight;
	for (int i = 0; i < mipMapCount; i++) {
		int mipsize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;
		outputFaceSize += mipsize;

		mipWidth /= 2;
		mipHeight /= 2;
	}

	int faceCount = isSixSidedCubemap ? 6 : 1;
	uint64_t outputTotalSize = outputFaceSize * faceCount;

	uint8_t* outData = new uint8_t[outputTotalSize];
	int minMipLevel = std::max(mipMapCount, 1);
	for (int faceIterator = 0; faceIterator < faceCount; faceIterator++) {
		GenerateFace(minMipLevel, faceIterator, blockSize, outData + (outputFaceSize * faceIterator));
	}

	OutputDds(outData, outputTotalSize);
}

uint64_t cubemapX[6] = {2, 0, 1, 1, 1, 3};
uint64_t cubemapY[6] = {1, 1, 0, 2, 1, 1};

uint8_t* TextureImporter::ExtractFirstFace(uint8_t faceIndex) {
	uint64_t sourcePitch = static_cast<uint64_t>(sourceWidth * texChannels);
	uint64_t targetPitch = static_cast<uint64_t>(texWidth * texChannels);
	uint64_t targetSize = static_cast<uint64_t>(targetPitch * texHeight);
	uint8_t* faceOutput = new uint8_t[targetSize];

	uint64_t firstPixelOffset = isSixSidedCubemap
		? (cubemapY[faceIndex] * texHeight * sourcePitch) + (cubemapX[faceIndex] * targetPitch)
		: 0;
	uint8_t* firstPixel = sourcePixels + firstPixelOffset;

	uint64_t dst = 0;
	for (uint64_t targetRow = 0; targetRow < texHeight; targetRow++) {
		for (uint64_t targetCol = 0; targetCol < texWidth; targetCol++) {
			uint64_t sourceIndex = (targetRow * sourcePitch) + (targetCol * texChannels);
			for (uint64_t channelIndex = 0; channelIndex < texChannels; ++channelIndex) {
				faceOutput[dst++] = CombinePixels(firstPixel, sourceIndex + channelIndex, sourcePitch);
			}
		}
	}

	return faceOutput;
}

void TextureImporter::GenerateMipList(uint8_t faceIndex, uint32_t minMipLevel, std::vector<uint8_t*>& uncompressedMips) {
	uint32_t levelWidth = texWidth;
	uint32_t levelHeight = texHeight;

	uncompressedMips.resize(minMipLevel);
	uncompressedMips[0] = isSixSidedCubemap
		? ExtractFirstFace(faceIndex)
		: sourcePixels;

#if false
	std::string fileName = isSixSidedCubemap
		? path.filename().replace_extension("").string() + "_face" + std::to_string(faceIndex)
		: path.filename().replace_extension("").string();
	std::string extension = ".tga"; //path.extension().string();
	std::filesystem::path mainFaceFilePath = std::filesystem::path("D:/MipGen") / (fileName + extension);
	stbi_write_tga(mainFaceFilePath.string().c_str(), levelWidth, levelHeight, texChannels, uncompressedMips[0]);
#endif
	for (uint32_t mipLevelIterator = 1; mipLevelIterator < minMipLevel; mipLevelIterator++) {
		levelWidth /= 2;
		levelHeight /= 2;

		uncompressedMips[mipLevelIterator] = CreateMip(uncompressedMips[mipLevelIterator - 1], levelWidth, levelHeight);

#if false
		std::filesystem::path faceMipFilePath = std::filesystem::path("D:/MipGen") / (fileName + "_mip" + std::to_string(mipLevelIterator) + extension);
		stbi_write_tga(faceMipFilePath.string().c_str(), levelWidth, levelHeight, texChannels, uncompressedMips[mipLevelIterator]);
#endif
	}
}

void TextureImporter::GenerateFace(uint32_t minMipLevel, uint32_t faceIterator, uint32_t blockSize, uint8_t* outData) {
	uint32_t dstOffset = 0;
	uint8_t block[64];

	std::vector<uint8_t*> uncompressedMips;
	GenerateMipList(faceIterator, minMipLevel, uncompressedMips);

	uint32_t levelWidth = texWidth;
	uint32_t levelHeight = texHeight;

	bool hasAlpha = texChannels == 4;
	for (uint32_t mipLevelIterator = 0; mipLevelIterator < minMipLevel; mipLevelIterator++) {
		uint8_t* mipSource = uncompressedMips[mipLevelIterator];

		for (uint32_t mipRow = 0; mipRow < levelHeight; mipRow += blockWidth) {
			for (uint32_t mipCol = 0; mipCol < levelWidth; mipCol += blockWidth) {
				uint8_t* ptr = mipSource + ((mipRow * levelWidth + mipCol) * texChannels);
				ExtractBlock(ptr, levelWidth, block);
				if (compression == Compression::BC4) {
					stb_compress_bc4_block(&outData[dstOffset], block);
				}
				else {
					stb_compress_dxt_block(outData + dstOffset, block, hasAlpha, STB_DXT_NORMAL);
				}
				dstOffset += blockSize;
			}
		}

		levelWidth /= 2;
		levelHeight /= 2;
	}
}

void TextureImporter::OutputDds(uint8_t* outData, uint64_t contentSize) {
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

	if (isSixSidedCubemap) {
		outHeader.dwCaps2 = DDS_CUBEMAP_ALLFACES;
	}

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
