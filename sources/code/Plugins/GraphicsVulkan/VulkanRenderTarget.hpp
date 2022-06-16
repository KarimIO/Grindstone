#pragma once

#include <Common/Graphics/RenderTarget.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderTarget : public RenderTarget {
		public:
			VulkanRenderTarget(VkImage swapchainImage, VkFormat); // Build from swapchain
			VulkanRenderTarget(RenderTarget::CreateInfo& ci);
			virtual ~VulkanRenderTarget() override;
		public:
			VkImageView GetImageView();
		public:
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) override;
		private:
			VkImage image;
			VkImageView imageView;
			VkDeviceMemory imageMemory;
		};
	}
}
