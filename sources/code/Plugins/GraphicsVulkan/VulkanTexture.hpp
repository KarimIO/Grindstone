#pragma once

#include <Common/Graphics/Texture.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanTextureBindingLayout : public TextureBindingLayout {
		public:
			VulkanTextureBindingLayout(TextureBindingLayout::CreateInfo& ci);
			~VulkanTextureBindingLayout();
			VkDescriptorSetLayout GetDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptorSetLayout;
		};

		class VulkanTexture : public Texture {
		public:
			VulkanTexture(Texture::CreateInfo& ci);
			virtual ~VulkanTexture();
		public:
			VkImageView GetImageView();
			VkSampler GetSampler();
			void CreateTextureImage(Texture::CreateInfo &ci, uint32_t &mipLevels);
			void CreateTextureSampler(Texture::CreateInfo &ci, uint32_t mipLevels);
			virtual void RecreateTexture(CreateInfo& createInfo) override;
		private:
			VkImage image;
			VkDeviceMemory imageMemory;
			VkImageView imageView;
			VkSampler sampler;
			VkFormat format;
		};

		class VulkanTextureBinding : public TextureBinding {
		public:
			VulkanTextureBinding(TextureBinding::CreateInfo& ci);
			~VulkanTextureBinding();
		public:
			VkDescriptorSet GetDescriptorSet();
		private:
			VkDescriptorSet descriptorSet;
		};
	};
};
