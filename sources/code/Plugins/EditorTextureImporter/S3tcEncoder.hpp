#pragma once

#include <vector>

#include "OutputFormats.hpp"

namespace Grindstone::Editor::Importers::Texture {
	void EncodeS3tc(
		uint8_t* dstPixels,
		size_t dstPixelsSize,
		const std::vector<uint8_t>& srcPixels,
		const OutputFormat outputFormat,
		const size_t width,
		const size_t height,
		const size_t channelCount
	);
}
