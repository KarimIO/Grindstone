#pragma once

#include <Common/Graphics/Image.hpp>
#include <Common/Graphics/Formats.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	VkFilter TranslateFilterToVulkan(TextureFilter);
	VkSamplerMipmapMode TranslateMipFilterToVulkan(TextureFilter);
	VkSamplerAddressMode TranslateWrapToVulkan(TextureWrapMode);
	VkFormat TranslateFormatToVulkan(Format format);
	Format TranslateFormatFromVulkan(VkFormat format);
	VkCullModeFlags TranslateCullModeToVulkan(CullMode cullMode);
	VkColorComponentFlags TranslateColorMaskToVulkan(ColorMask colorMask);
	VkPolygonMode TranslatePolygonModeToVulkan(PolygonFillMode mode);
	VkPrimitiveTopology TranslateGeometryTypeToVulkan(GeometryType geometryType);

	VkBlendOp TranslateBlendOpToVulkan(BlendOperation op);
	VkBlendFactor TranslateBlendFactorToVulkan(BlendFactor factor);
	VkCompareOp TranslateCompareOpToVulkan(CompareOperation op);
}
