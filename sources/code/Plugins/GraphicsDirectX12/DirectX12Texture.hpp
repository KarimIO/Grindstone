#pragma once

#include "../GraphicsCommon/Texture.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12TextureBindingLayout : public TextureBindingLayout {
		public:
			DirectX12TextureBindingLayout(TextureBindingLayoutCreateInfo ci);
			~DirectX12TextureBindingLayout() {};
			//VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			//VkDescriptorSetLayout descriptor_set_layout_;
		};

		class DirectX12Texture : public Texture {
		public:
			DirectX12Texture(TextureCreateInfo ci);
			virtual ~DirectX12Texture() {};
		public:
			//VkImageView getImageView();
			//VkSampler getSampler();
			void createTextureImage(TextureCreateInfo &ci, uint32_t &mipLevels);
			void createTextureSampler(TextureCreateInfo &ci, uint32_t mipLevels);
		private:
			/*VkImage image_;
			VkDeviceMemory image_memory_;
			VkImageView image_view_;
			VkSampler sampler_;
			VkFormat format_;*/
		};

		class DirectX12TextureBinding : public TextureBinding {
		public:
			DirectX12TextureBinding(TextureBindingCreateInfo ci);
			~DirectX12TextureBinding() {};
		public:
			//VkDescriptorSet getDescriptorSet();
		private:
			//VkDescriptorSet descriptor_set_;
		};
	};
};
