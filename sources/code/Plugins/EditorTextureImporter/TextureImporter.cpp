#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>

#include <EngineCore/Assets/AssetManager.hpp>
#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Common/Formats/Dds.hpp>
#include <Editor/EditorManager.hpp>

#include "Bc6Encoder.hpp"
#include "S3tcEncoder.hpp"
#include "DdsFileWriter.hpp"

#include "TextureImporter.hpp"

using namespace Grindstone::Editor::Importers;

const uint32_t blockWidth = 4;

uint32_t CalculateMipMapLevelCount(uint32_t width, uint32_t height) {
	uint32_t size = std::min(width, height);
	return static_cast<uint32_t>(std::log2(size) - 1);
}

template<typename SourcePixelType, typename LargerPixelType>
static inline uint8_t BilinearDownsamplePixelPerChannel(const uint8_t* pixelSrc, const uint64_t index, const uint64_t pitch, const LargerPixelType averageMultiplier) {
	LargerPixelType pixelA = static_cast<LargerPixelType>(*reinterpret_cast<SourcePixelType*>(pixelSrc + index));
	LargerPixelType pixelB = static_cast<LargerPixelType>(*reinterpret_cast<SourcePixelType*>(pixelSrc + (index + texChannels)));
	LargerPixelType pixelC = static_cast<LargerPixelType>(*reinterpret_cast<SourcePixelType*>(pixelSrc + (index + pitch)));
	LargerPixelType pixelD = static_cast<LargerPixelType>(*reinterpret_cast<SourcePixelType*>(pixelSrc + (index + pitch + texChannels)));
	LargerPixelType average = (pixelA + pixelB + pixelC + pixelD) / averageMultiplier;
	return static_cast<SourcePixelType>(average);
}

template<typename SourcePixelType, typename LargerPixelType>
void CreateMip(uint8_t* srcPixel, uint8_t* dstPixel, uint32_t dstWidth, uint32_t dstHeight) {
	uint64_t pitch = static_cast<uint64_t>(dstWidth * texChannels);
	uint64_t size = static_cast<uint64_t>(pitch * dstHeight);
	uint64_t dst = 0;
	for (uint64_t srcRow = 0; srcRow < dstHeight; srcRow++) {
		for (uint64_t srcCol = 0; srcCol < dstWidth; srcCol++) {
			uint64_t index = (srcRow * pitch) + (srcCol * texChannels);
			uint8_t* src = srcPixel + index;
			for (uint64_t channelIndex = 0; channelIndex < texChannels; ++channelIndex) {
				dstPixel[dst++] = BilinearDownsamplePixelPerChannel<SourcePixelType, LargerPixelType>(src, index + channelIndex, pitch, 4);
			}
		}
	}
}

static void CreateMipChain(
	uint8_t* sourcePixels,
	std::vector<uint8_t>& mipchainData,
	std::vector<uint8_t*>& mipsPtrs,
	uint32_t texWidth,
	uint32_t texHeight,
	SourceChannelDataType dataType,
	bool shouldGenerateMips
) {
	uint32_t mipMapCount = shouldGenerateMips
		? CalculateMipMapLevelCount(texWidth, texHeight)
		: 1;

	uint64_t outputSize = 0;
	uint32_t mipWidth = texWidth;
	uint32_t mipHeight = texHeight;
	uint32_t pixelSize = (dataType == SourceChannelDataType::Uint8)
		? sizeof(uint8_t)
		: sizeof(float);
	pixelSize *= 4;

	for (uint32_t i = 0; i < mipMapCount; i++) {
		uint32_t mipsize = mipWidth * mipHeight * pixelSize;
		outputSize += mipsize;

		mipWidth /= 2;
		mipHeight /= 2;
	}

	for (uint32_t i = 0; i < mipMapCount; i++) {
	}
}

static void EncodeMipChain(
	OutputFormat outputFormat,
	std::vector<uint8_t*> mipsPtrs,
	uint8_t* outputData,
	uint32_t width,
	uint32_t height,
	uint32_t channelCount
) {
	switch (outputFormat) {
		case OutputFormat::BC1:
		case OutputFormat::BC3:
		case OutputFormat::BC4:
			Texture::EncodeS3tc(outputData, outputDataSize, mipsPtr, outputFormat, width, height, channelCount);
			break;
		case OutputFormat::BC6H:
			Texture::EncodeBc6h(outputData, outputDataSize, mipsPtr, width, height);
			break;
		case OutputFormat::Undefined:
			break;
	}
}

const uint64_t cubemapX[6] = { 2, 0, 1, 1, 1, 3 };
const uint64_t cubemapY[6] = { 1, 1, 0, 2, 1, 1 };

uint8_t* ExtractFirstFace(uint8_t faceIndex) {
	uint64_t sourcePitch = static_cast<uint64_t>(sourceWidth * texChannels * sourceSizePerPixel);
	uint64_t targetPitch = static_cast<uint64_t>(texWidth * texChannels * sourceSizePerPixel);
	uint64_t targetSize = static_cast<uint64_t>(targetPitch * texHeight);
	uint8_t* faceOutput = new uint8_t[targetSize];

	uint64_t firstPixelOffset = isSixInOneImageCubemap
		? (cubemapY[faceIndex] * texHeight * sourcePitch) + (cubemapX[faceIndex] * targetPitch)
		: 0;
	uint8_t* firstPixel = sourcePixels + firstPixelOffset;

	uint64_t dst = 0;
	for (uint64_t targetRow = 0; targetRow < texHeight; targetRow++) {
		uint64_t rowIndex = (targetRow * sourcePitch);
		for (uint64_t targetCol = 0; targetCol < texWidth; targetCol++) {
			uint64_t sourceIndex = rowIndex + (targetCol * texChannels);
			for (uint64_t channelIndex = 0; channelIndex < texChannels; ++channelIndex) {
				faceOutput[dst++] = BilinearDownsamplePixelPerChannel(firstPixel, sourceIndex + channelIndex, sourcePitch);
			}
		}
	}

	return faceOutput;
}

void TextureImporter::Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) {
	this->path = path;
	this->assetRegistry = &assetRegistry;
	this->assetManager = &assetManager;
	int width = 0, height = 0, channels = 0;

	bool isHDR = path.extension() == ".hdr";
	if (isHDR) {
		sourcePixels = reinterpret_cast<uint8_t*>(stbi_loadf(path.string().c_str(), &width, &height, &channels, 0));
		sourceSizePerPixel = 2;
	}
	else {
		sourcePixels = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
		sourceSizePerPixel = 1;
	}

	SourceChannelDataType dataType = isHDR
		? SourceChannelDataType::Float
		: SourceChannelDataType::Uint8;

	if (!sourcePixels) {
		throw std::runtime_error("Unable to load texture!");
	}

	sourceWidth = static_cast<uint32_t>(width);
	sourceHeight = static_cast<uint32_t>(height);
	texChannels = static_cast<uint32_t>(channels);
	targetTexChannels = texChannels;

	// TODO: When adding options, make this an option. Not every 4:3 texture is a cubemap!
	isSixInOneImageCubemap = (width != height && (width / 4 == height / 3));
	texWidth = isSixInOneImageCubemap ? sourceWidth / 4 : sourceWidth;
	texHeight = isSixInOneImageCubemap ? sourceHeight / 3 : sourceHeight;

	metaFile = assetRegistry.GetMetaFileByPath(path);

	shouldGenerateMips = true;
	if (isHDR) {
		outputFormat = OutputFormat::BC6H;
	}
	else if (texChannels == 1) {
		outputFormat = OutputFormat::BC4;
	}
	else if (texChannels == 3) {
		outputFormat = OutputFormat::BC1;
	}
	else if (texChannels == 4) {
		outputFormat = OutputFormat::BC3;
	}
	else {
		outputFormat = OutputFormat::BC3;
		throw std::runtime_error("Invalid compression type!");
	}

	std::vector<uint8_t*> mipsPtrs;
	std::vector<uint8_t> mipchainData;
	CreateMipChain(sourcePixels, mipchainData, mipsPtrs, width, height, dataType, shouldGenerateMips);
	stbi_image_free(sourcePixels);

	EncodeMipChain(mipsPtrs, outputFileContents);

	DdsFileWriterOptions options{};
	options.texWidth = width;
	options.texHeight = height;
	options.texDepth = 1;
	options.isCubemap = isSixInOneImageCubemap;
	options.mipmapCount = mipsPtrs.size();
	options.outputFormat = outputFormat;

	std::filesystem::path parentPath = assetRegistry.GetCompiledAssetsPath();
	std::filesystem::path outputPath = parentPath / uuid.ToString();
	WriteDdsFile(outputPath, outputFileContents, options);
}

Grindstone::Uuid TextureImporter::GetUuid() const {
	return uuid;
}

void Grindstone::Editor::Importers::ImportTexture(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& inputPath) {
	TextureImporter TextureImporter;
	TextureImporter.Import(assetRegistry, assetManager, inputPath);
}
