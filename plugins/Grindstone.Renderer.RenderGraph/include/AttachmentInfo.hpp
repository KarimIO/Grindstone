#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Image.hpp>

namespace Grindstone::Renderer {
	enum class SizeUnit {
		SwapchainRelative,
		Absolute
	};

	struct AttachmentInfo {
		SizeUnit widthUnit;
		float width;
		SizeUnit heightUnit;
		float height;
		float depth;
		Grindstone::GraphicsAPI::Format format;
		Grindstone::GraphicsAPI::ImageUsageFlags usageFlags;
		std::string sizeRelativeName;
		unsigned samples = 1;
		unsigned levels = 1;
		unsigned layers = 1;
		bool persistent = true;
	};
}
