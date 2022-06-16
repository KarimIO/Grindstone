#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUtils.hpp"
#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanDepthTarget::VulkanDepthTarget(DepthTarget::CreateInfo& ci) {
			VkFormat depthFormat = TranslateDepthFormatToVulkan(ci.format);

			CreateImage(ci.width, ci.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
			imageView = CreateImageView(image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
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
	}
}
