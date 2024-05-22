#pragma once

#include <string>
#include <Common/Graphics/RenderTarget.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderTarget : public RenderTarget {
		public:
			VulkanRenderTarget(VkImage swapchainImage, VkFormat format, uint32_t swapchainIndex); // Build from swapchain
			VulkanRenderTarget(VkImage image, VkImageView imageView, VkFormat colorFormat); // Built from ImGui
			VulkanRenderTarget(RenderTarget::CreateInfo& createInfo);
			virtual ~VulkanRenderTarget() override;
		public:
			void UpdateNativeImage(VkImage image, VkImageView imageView, VkFormat format);
			void UpdateSwapChainImage(VkImage swapchainImage);
			VkImage GetImage() const;
			VkImageView GetImageView() const;
			VkSampler GetSampler() const;
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
			bool isWrittenByCompute = false;
			bool hasMipChain = false;
			bool isOwnedBySwapchain = false;
		};
	}
}
