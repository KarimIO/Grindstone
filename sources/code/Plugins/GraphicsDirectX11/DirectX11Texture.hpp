#pragma once

#include "../GraphicsCommon/Texture.hpp"
#include <d3d11.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11TextureBindingLayout : public TextureBindingLayout {
		public:
			DirectX11TextureBindingLayout(TextureBindingLayoutCreateInfo ci);
			~DirectX11TextureBindingLayout();
			//VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			//VkDescriptorSetLayout descriptor_set_layout_;
		};

		class DirectX11Texture : public Texture {
		public:
			DirectX11Texture(TextureCreateInfo ci);
			virtual ~DirectX11Texture();

			ID3D11ShaderResourceView* getSrv();
			ID3D11SamplerState* getSampler();
		private:
			ID3D11ShaderResourceView* texture_srv_;
			ID3D11SamplerState* sampler_state_;
		};

		class DirectX11TextureBinding : public TextureBinding {
		public:
			DirectX11TextureBinding(TextureBindingCreateInfo ci);
			~DirectX11TextureBinding();

			void bind();
		private:
			DirectX11Texture *texture_;
			//VkDescriptorSet descriptor_set_;
		};
	};
};
