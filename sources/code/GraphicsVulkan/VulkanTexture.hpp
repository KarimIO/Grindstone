#pragma once

#include "../GraphicsCommon/Texture.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanTextureBindingLayout : public TextureBindingLayout {
		public:
			VulkanTextureBindingLayout(TextureBindingLayoutCreateInfo ci);
			~VulkanTextureBindingLayout();
			VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptor_set_layout_;
		};

		class VulkanTexture : public Texture {
		public:
			VulkanTexture(TextureCreateInfo ci);
			virtual ~VulkanTexture();
		public:
			VkImageView getImageView();
			VkSampler getSampler();
			void createTextureImage(TextureCreateInfo &ci, uint32_t &mipLevels);
			void createTextureSampler(TextureCreateInfo &ci, uint32_t mipLevels);
		private:
			VkImage image_;
			VkDeviceMemory image_memory_;
			VkImageView image_view_;
			VkSampler sampler_;
			VkFormat format_;
		};

		class VulkanTextureBinding : public TextureBinding {
		public:
			VulkanTextureBinding(TextureBindingCreateInfo ci);
			~VulkanTextureBinding();
		public:
			VkDescriptorSet getDescriptorSet();
		private:
			VkDescriptorSet descriptor_set_;
		};
	};
};
