#include "VulkanRenderTarget.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <iostream>
#include <cassert>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanRenderTarget::VulkanRenderTarget(VkImage swapchainImage, VkFormat format) : image(swapchainImage) {
			imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		VulkanRenderTarget::VulkanRenderTarget(RenderTarget::CreateInfo& createInfo) {
			uint8_t channels;
			VkFormat renderFormat = TranslateColorFormatToVulkan(createInfo.format, channels);

			CreateImage(createInfo.width, createInfo.height, 1, renderFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
			imageView = CreateImageView(image, renderFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		VulkanRenderTarget::~VulkanRenderTarget() {
			VkDevice device = VulkanCore::Get().GetDevice();
			vkDestroyImageView(device, imageView, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, imageMemory, nullptr);
		}

		VkImageView VulkanRenderTarget::GetImageView() {
			return imageView;
		}

		void VulkanRenderTarget::Resize(uint32_t width, uint32_t height) {
		}

		void VulkanRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
			std::cout << "VulkanRenderTarget::RenderScreen is not used.\n";
			assert(false);
		}

	}
}
