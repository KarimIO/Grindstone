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
#include "S3tcEncoder.hpp"

using namespace Grindstone::Editor::Importers;

const size_t blockWidth = 4;
const size_t pixelChannelSize = 1;

static void S3tcExtractBlock(
	const uint8_t* inPtr,
	const size_t levelWidth,
	const size_t channelCount,
	uint8_t* colorBlock
) {
	const size_t pixelSize = channelCount * pixelChannelSize;

	// stb_dxt requires channel sizes of 4, so pad it to 4 and set the alpha channel to 255.
	if (channelCount == 3) {
		for (uint32_t dstRow = 0; dstRow < blockWidth; dstRow++) {
			for (uint32_t dstCol = 0; dstCol < blockWidth; dstCol++) {
				memcpy(colorBlock + (dstRow * blockWidth + dstCol) * 4 * pixelChannelSize, inPtr + (dstRow * levelWidth + dstCol) * pixelSize, pixelSize);
				colorBlock[(dstRow * blockWidth + dstCol) * 4 + 3] = 255;
			}
		}
		return;
	}

	for (uint32_t dstRow = 0; dstRow < blockWidth; dstRow++) {
		for (uint32_t dstCol = 0; dstCol < blockWidth; dstCol++) {
			memcpy(colorBlock + (dstRow * blockWidth + dstCol) * pixelSize, inPtr + (dstRow * levelWidth + dstCol) * pixelSize, pixelSize);
		}
	}
}

void Texture::EncodeS3tc(
	uint8_t* dstPixels,
	size_t dstPixelsSize,
	const std::vector<uint8_t>& srcPixels,
	const OutputFormat outputFormat,
	const size_t width,
	const size_t height,
	const size_t channelCount
) {
	uint64_t outputFaceSize = 0;
	size_t blockSize = (outputFormat == OutputFormat::BC3) ? 16 : 8;
	size_t requiredSize = ((width + 3u) / 4u) * ((height + 3u) / 4u) * blockSize;

	if (requiredSize < dstPixelsSize) {
		// TODO: Throw an error?
		return;
	}

	std::array<uint8_t, 64> currentEncodeBlock{};
	bool hasAlpha = channelCount == 4;
	size_t dstOffset = 0;
	for (size_t blockRow = 0; blockRow < width; blockRow += blockWidth) {
		for (size_t blockCol = 0; blockCol < height; blockCol += blockWidth) {
			const uint8_t* ptr = srcPixels.data() + ((blockRow * width + blockCol) * channelCount);
			S3tcExtractBlock(ptr, width, channelCount, currentEncodeBlock.data());

			if (outputFormat == OutputFormat::BC4) {
				stb_compress_bc4_block(&dstPixels[dstOffset], currentEncodeBlock.data());
			}
			else {
				stb_compress_dxt_block(&dstPixels[dstOffset], currentEncodeBlock.data(), hasAlpha, STB_DXT_NORMAL);
			}

			dstOffset += blockSize;
		}
	}
}
