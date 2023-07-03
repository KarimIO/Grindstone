#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUtils.hpp"
#include <assert.h>

using namespace Grindstone::GraphicsAPI;

VulkanDepthTarget::VulkanDepthTarget(DepthTarget::CreateInfo& createInfo) {
	VkFormat depthFormat = TranslateDepthFormatToVulkan(createInfo.format);

	CreateImage(createInfo.width, createInfo.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	imageView = CreateImageView(image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1);

	std::string debugName = createInfo.debugName;
	std::string imageViewDebugName = debugName + " View";
	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, createInfo.debugName);
	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
}
		
VulkanDepthTarget::~VulkanDepthTarget() {
	VkDevice device = VulkanCore::Get().GetDevice();
	vkDestroyImageView(device, imageView, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}

VkImageView VulkanDepthTarget::GetImageView() {
	return imageView;
}

void VulkanDepthTarget::Resize(uint32_t width, uint32_t height) {
	std::cout << "VulkanDepthTarget::BindFace is not used.\n";
	assert(false);
}

void VulkanDepthTarget::BindFace(int k) {
	std::cout << "VulkanDepthTarget::BindFace is not used.\n";
	assert(false);
}
