#pragma once

#include <Common/Graphics/RenderTarget.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderTarget : public RenderTarget {
		public:
			VulkanRenderTarget(VkImage swapchainImage, VkFormat format); // Build from swapchain
			VulkanRenderTarget(RenderTarget::CreateInfo& createInfo);
			virtual ~VulkanRenderTarget() override;
		public:
			VkImageView GetImageView();
		public:
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void RenderScreen(unsigned int i, unsigned int width, unsigned int height, unsigned char *data) override;
		private:
			VkImage image = nullptr;
			VkImageView imageView = nullptr;
			VkDeviceMemory imageMemory = nullptr;
		};
	}
}
