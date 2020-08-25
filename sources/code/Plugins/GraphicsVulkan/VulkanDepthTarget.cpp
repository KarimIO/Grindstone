#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUtils.hpp"
#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanDepthTarget::VulkanDepthTarget(DepthTarget::CreateInfo& ci) {
			VkFormat depthFormat = TranslateDepthFormatToVulkan(ci.format);

			createImage(ci.width, ci.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
			image_view_ = createImageView(image_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		}
		
		VulkanDepthTarget::~VulkanDepthTarget() {
			VkDevice device = VulkanCore::get().getDevice();
			vkDestroyImageView(device, image_view_, nullptr);
			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		VkImageView VulkanDepthTarget::getImageView() {
			return image_view_;
		}
		void VulkanDepthTarget::BindFace(int k) {
			std::cout << "VulkanDepthTarget::BindFace is not used.\n";
			assert(false);
		}
	}
}
