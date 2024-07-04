#pragma once

#include "OutputFormats.hpp"

namespace Grindstone::Editor::Importers::Texture {
	void EncodeBc6h(
		uint8_t* dstPixels,
		size_t dstPixelsSize,
		const std::vector<uint8_t>& srcPixels,
		const size_t width,
		const size_t height
	);
}
