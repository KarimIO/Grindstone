#pragma once

#include <Common/Graphics/Texture.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class Texture : public Grindstone::GraphicsAPI::Texture {
	public:
		Texture(const CreateInfo& createInfo);
		virtual ~Texture();
	public:
		virtual VkImageView GetImageView() const;
		VkSampler GetSampler() const;
		virtual void RecreateTexture(const CreateInfo& createInfo) override;
	private:
		void CreateTextureImage(const CreateInfo &createInfo, uint32_t &mipLevels, uint32_t layerCount);
		void CreateTextureSampler(const CreateInfo &createInfo, uint32_t mipLevels);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		VkDeviceMemory imageMemory = nullptr;
		VkImageView imageView = nullptr;
		VkSampler sampler = nullptr;
		VkImage image = nullptr;
		VkFormat format = VK_FORMAT_UNDEFINED;
	};
}
