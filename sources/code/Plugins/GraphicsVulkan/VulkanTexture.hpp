#pragma once

#include <Common/Graphics/Texture.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanTextureBindingLayout : public TextureBindingLayout {
		public:
			VulkanTextureBindingLayout(TextureBindingLayout::CreateInfo& createInfo);
			~VulkanTextureBindingLayout();
			VkDescriptorSetLayout GetDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptorSetLayout = nullptr;
		};

		class VulkanTexture : public Texture {
		public:
			VulkanTexture(Texture::CreateInfo& createInfo);
			virtual ~VulkanTexture();
		public:
			VkImageView GetImageView();
			VkSampler GetSampler();
			void CreateTextureImage(Texture::CreateInfo &createInfo, uint32_t &mipLevels);
			void CreateTextureSampler(Texture::CreateInfo &createInfo, uint32_t mipLevels);
			virtual void RecreateTexture(CreateInfo& createInfo) override;
		private:
			VkDeviceMemory imageMemory = nullptr;
			VkImageView imageView = nullptr;
			VkSampler sampler = nullptr;
			VkImage image = nullptr;
			VkFormat format = VK_FORMAT_UNDEFINED;
		};

		class VulkanTextureBinding : public TextureBinding {
		public:
			VulkanTextureBinding(TextureBinding::CreateInfo& createInfo);
			~VulkanTextureBinding();
		public:
			VkDescriptorSet GetDescriptorSet();
		private:
			VkDescriptorSet descriptorSet = nullptr;
		};
	};
};
