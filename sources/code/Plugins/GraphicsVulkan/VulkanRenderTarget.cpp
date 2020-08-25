#include "VulkanRenderTarget.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <iostream>
#include <cassert>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanRenderTarget::VulkanRenderTarget(VkImage swapchain_image, VkFormat format) : image_(swapchain_image) {
			image_view_ = createImageView(image_, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		VulkanRenderTarget::VulkanRenderTarget(RenderTarget::CreateInfo& ci) {
			uint8_t channels;
			VkFormat render_format = TranslateColorFormatToVulkan(ci.format, channels);

			createImage(ci.width, ci.height, 1, render_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
			image_view_ = createImageView(image_, render_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		VulkanRenderTarget::~VulkanRenderTarget() {
			VkDevice device = VulkanCore::get().getDevice();
			vkDestroyImageView(device, image_view_, nullptr);
			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		VkImageView VulkanRenderTarget::getImageView() {
			return image_view_;
		}

		float VulkanRenderTarget::getAverageValue(uint32_t i) {
			std::cout << "VulkanRenderTarget::getAverageValue is not used.\n";
			assert(false);
			return 0.0f;
		}

		void VulkanRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
			std::cout << "VulkanRenderTarget::RenderScreen is not used.\n";
			assert(false);
		}

	}
}