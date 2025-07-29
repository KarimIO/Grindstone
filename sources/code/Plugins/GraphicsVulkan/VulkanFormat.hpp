#pragma once

#include <Common/Graphics/Image.hpp>
#include <Common/Graphics/Formats.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	VkImageAspectFlags TranslateImageAspectBitsToVulkan(ImageAspectBits imageAspectMask);
	VkPipelineStageFlagBits TranslatePipelineStageToVulkan(PipelineStageBit stage);
	VkFilter TranslateFilterToVulkan(TextureFilter);
	VkSamplerMipmapMode TranslateMipFilterToVulkan(TextureFilter);
	VkSamplerAddressMode TranslateWrapToVulkan(TextureWrapMode);
	VkFormat TranslateFormatToVulkan(Format format);
	Format TranslateFormatFromVulkan(VkFormat format);
	VkCullModeFlags TranslateCullModeToVulkan(CullMode cullMode);
	VkColorComponentFlags TranslateColorMaskToVulkan(ColorMask colorMask);
	VkPolygonMode TranslatePolygonModeToVulkan(PolygonFillMode mode);
	VkPrimitiveTopology TranslateGeometryTypeToVulkan(GeometryType geometryType);
	VkImageLayout TranslateImageLayoutToVulkan(Grindstone::GraphicsAPI::ImageLayout layout);

	VkBlendOp TranslateBlendOpToVulkan(BlendOperation op);
	VkBlendFactor TranslateBlendFactorToVulkan(BlendFactor factor);
	VkCompareOp TranslateCompareOpToVulkan(CompareOperation op);
}
