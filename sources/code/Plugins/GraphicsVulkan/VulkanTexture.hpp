#pragma once

#include <Common/Graphics/Texture.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanTexture : public Texture {
		public:
			VulkanTexture(Texture::CreateInfo& createInfo);
			virtual ~VulkanTexture();
		public:
			virtual VkImageView GetImageView();
			VkSampler GetSampler();
			virtual void RecreateTexture(CreateInfo& createInfo) override;
		private:
			void CreateTextureImage(Texture::CreateInfo &createInfo, uint32_t &mipLevels);
			void CreateTextureSampler(Texture::CreateInfo &createInfo, uint32_t mipLevels);
			void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

			VkDeviceMemory imageMemory = nullptr;
			VkImageView imageView = nullptr;
			VkSampler sampler = nullptr;
			VkImage image = nullptr;
			VkFormat format = VK_FORMAT_UNDEFINED;
		};
	};
};
