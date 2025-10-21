#pragma once

#include <stdint.h>

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Image.hpp>
#include <Common/Containers/Span.hpp>

#include "Dds.hpp"

namespace Grindstone::Formats::DDS {
	struct DdsParseOutput {
		bool isCubemap = false;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 0;
		uint32_t mipLevels = 0;
		uint32_t arraySize = 0;
		Grindstone::GraphicsAPI::ImageDimension dimensions;
		Grindstone::GraphicsAPI::Format format;
		Grindstone::Containers::BufferSpan data;
	};

	bool TryParseDds(const char* debugName, Grindstone::Containers::BufferSpan bufferView, DdsParseOutput& output);
	uint64_t GetRequiredBytes(const DdsParseOutput& ddsData);
}
