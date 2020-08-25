#pragma once

#include <Common/Graphics/Texture.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanTextureBindingLayout : public TextureBindingLayout {
		public:
			VulkanTextureBindingLayout(TextureBindingLayout::CreateInfo& ci);
			~VulkanTextureBindingLayout();
			VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptor_set_layout_;
		};

		class VulkanTexture : public Texture {
		public:
			VulkanTexture(Texture::CreateInfo& ci);
			virtual ~VulkanTexture();
		public:
			VkImageView getImageView();
			VkSampler getSampler();
			void createTextureImage(Texture::CreateInfo &ci, uint32_t &mipLevels);
			void createTextureSampler(Texture::CreateInfo &ci, uint32_t mipLevels);
		private:
			VkImage image_;
			VkDeviceMemory image_memory_;
			VkImageView image_view_;
			VkSampler sampler_;
			VkFormat format_;
		};

		class VulkanTextureBinding : public TextureBinding {
		public:
			VulkanTextureBinding(TextureBinding::CreateInfo& ci);
			~VulkanTextureBinding();
		public:
			VkDescriptorSet getDescriptorSet();
		private:
			VkDescriptorSet descriptor_set_;
		};
	};
};
