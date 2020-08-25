#pragma once

#include <Common/Graphics/RenderTarget.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderTarget : public RenderTarget {
		public:
			VulkanRenderTarget(VkImage swapchain_image, VkFormat); // Build from swapchain
			VulkanRenderTarget(RenderTarget::CreateInfo& ci);
			virtual ~VulkanRenderTarget() override;
		public:
			VkImageView getImageView();
		public:
			virtual float getAverageValue(uint32_t i) override;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) override;
		private:
			VkImage image_;
			VkImageView image_view_;
			VkDeviceMemory image_memory_;
		};
	}
}
