#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanUtils.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanSampler.hpp>

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::Sampler::Sampler(const CreateInfo& createInfo) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = TranslateFilterToVulkan(createInfo.options.magFilter);
	samplerInfo.minFilter = TranslateFilterToVulkan(createInfo.options.minFilter);
	samplerInfo.addressModeU = TranslateWrapToVulkan(createInfo.options.wrapModeU);
	samplerInfo.addressModeV = TranslateWrapToVulkan(createInfo.options.wrapModeV);
	samplerInfo.addressModeW = TranslateWrapToVulkan(createInfo.options.wrapModeW);
	samplerInfo.anisotropyEnable = (createInfo.options.anistropy != 0) ? VK_TRUE : VK_FALSE;

	// TODO: anisotropy must be between 1 and device limit
	samplerInfo.maxAnisotropy = createInfo.options.anistropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.mipmapMode = TranslateMipFilterToVulkan(createInfo.options.mipFilter);
	samplerInfo.minLod = createInfo.options.mipMin;
	samplerInfo.maxLod = createInfo.options.mipMax;
	samplerInfo.mipLodBias = createInfo.options.mipBias;

	if (vkCreateSampler(Vulkan::Core::Get().GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create texture sampler!");
	}

	if (createInfo.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_SAMPLER, sampler, createInfo.debugName);
	}
	else {
		GPRINT_WARN(LogSource::GraphicsAPI, "Unnamed sampler!");
	}
}

Vulkan::Sampler::~Sampler() {
	vkDestroySampler(Vulkan::Core::Get().GetDevice(), sampler, nullptr);
}

VkSampler Vulkan::Sampler::GetSampler() const {
	return sampler;
}
