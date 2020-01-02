#pragma once

#include "../GraphicsCommon/Texture.hpp"
#include "../GraphicsCommon/Formats.hpp"
#include "../GraphicsCommon/VertexBuffer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VkFilter TranslateFilterToVulkan(TextureFilter);
		VkSamplerAddressMode TranslateWrapToVulkan(TextureWrapMode);
		VkFormat TranslateVertexFormatsToVulkan(VertexFormat format);
		ColorFormat TranslateColorFormatFromVulkan(VkFormat format);
		DepthFormat TranslateDepthFormatFromVulkan(VkFormat format);
		VkFormat TranslateColorFormatToVulkan(ColorFormat, uint8_t &channels);
		VkFormat TranslateDepthFormatToVulkan(DepthFormat);
	}
}