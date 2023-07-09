#pragma once

#include <string>
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
			void UpdateSwapChainImage(VkImage swapchainImage);
			VkImageView GetImageView();
			VkSampler GetSampler();
		public:
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void RenderScreen(unsigned int i, unsigned int width, unsigned int height, unsigned char *data) override;
		private:
			void CreateTextureSampler();
			void Cleanup();
			void Create();
		private:
			VkSampler sampler = nullptr;
			VkImage image = nullptr;
			VkImageView imageView = nullptr;
			VkDeviceMemory imageMemory = nullptr;

			std::string debugName;
			ColorFormat format = ColorFormat::Invalid;
			uint32_t width = 0;
			uint32_t height = 0;
			bool isSampled = false;
			bool isOwnedBySwapchain = false;
		};
	}
}
