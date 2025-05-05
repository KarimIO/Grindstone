#pragma once

#include <Common/Graphics/Texture.hpp>
#include <Common/Graphics/Formats.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	VkFilter TranslateFilterToVulkan(TextureFilter);
	VkSamplerMipmapMode TranslateMipFilterToVulkan(TextureFilter);
	VkSamplerAddressMode TranslateWrapToVulkan(TextureWrapMode);
	VkFormat TranslateVertexFormatsToVulkan(VertexFormat format);
	ColorFormat TranslateColorFormatFromVulkan(VkFormat format);
	DepthFormat TranslateDepthFormatFromVulkan(VkFormat format);
	VkFormat TranslateColorFormatToVulkan(ColorFormat, uint8_t &channels);
	VkFormat TranslateDepthFormatToVulkan(DepthFormat, bool& hasStencil);
	VkCullModeFlags TranslateCullModeToVulkan(CullMode cullMode);
	VkColorComponentFlags TranslateColorMaskToVulkan(ColorMask colorMask);
	VkPolygonMode TranslatePolygonModeToVulkan(PolygonFillMode mode);
	VkPrimitiveTopology TranslateGeometryTypeToVulkan(GeometryType geometryType);

	VkBlendOp TranslateBlendOpToVulkan(BlendOperation op);
	VkBlendFactor TranslateBlendFactorToVulkan(BlendFactor factor);
	VkCompareOp TranslateCompareOpToVulkan(CompareOperation op);
}
