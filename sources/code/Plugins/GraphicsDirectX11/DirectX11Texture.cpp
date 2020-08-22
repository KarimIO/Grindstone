#include "DirectX11Texture.hpp"
#include "DirectX11Utils.hpp"
#include "DirectX11Format.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11TextureBindingLayout::DirectX11TextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
			//d3d11DevCon->PSSetSamplers(0, 1, &CubesTexSamplerState);
		}

		DirectX11TextureBindingLayout::~DirectX11TextureBindingLayout() {
		}

		DirectX11Texture::DirectX11Texture(TextureCreateInfo ci) {
			auto device = DirectX11GraphicsWrapper::get().device_;
			HRESULT hr;
			
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

			D3D11_TEXTURE2D_DESC desc;
			desc.Width = ci.width;
			desc.Height = ci.height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			UINT rowPitch = ci.width * 4;

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = ci.data;
			initData.SysMemPitch = rowPitch;
			initData.SysMemSlicePitch = rowPitch * ci.height;

			ID3D11Texture2D* tex = nullptr;
			hr = device->CreateTexture2D(&desc, &initData, &tex);

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
			memset(&SRVDesc, 0, sizeof(SRVDesc));
			SRVDesc.Format = format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;

			hr = device->CreateShaderResourceView(tex, &SRVDesc, &texture_srv_);
			if (FAILED(hr)) {
				tex->Release();
				return;
			}

			D3D11_FILTER filter[] = { D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_LINEAR };
			/*Nearest = 0,
				Linear,
				NearestMipMapNearest,
				NearestMipMapLinear,
				LinearMipMapNearest,
				LinearMipMapLinear*/

			static D3D11_TEXTURE_ADDRESS_MODE addrmode[] = {
				D3D11_TEXTURE_ADDRESS_WRAP,
				D3D11_TEXTURE_ADDRESS_CLAMP,
				D3D11_TEXTURE_ADDRESS_BORDER,
				D3D11_TEXTURE_ADDRESS_MIRROR,
				D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
			};

			D3D11_SAMPLER_DESC sampDesc;
			ZeroMemory(&sampDesc, sizeof(sampDesc));
			//ci.options.min_filter * 6 + ci.options.mag_filter
			sampDesc.Filter = filter[0];
			sampDesc.AddressU = addrmode[(uint8_t)ci.options.wrap_mode_u];
			sampDesc.AddressV = addrmode[(uint8_t)ci.options.wrap_mode_v];
			sampDesc.AddressW = addrmode[(uint8_t)ci.options.wrap_mode_w];
			sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampDesc.MinLOD = 0;
			sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

			if (device->CreateSamplerState(&sampDesc, &sampler_state_) < 0)
				return;
		}

		ID3D11ShaderResourceView* DirectX11Texture::getSrv() {
			return texture_srv_;
		}

		ID3D11SamplerState* DirectX11Texture::getSampler() {
			return sampler_state_;
		}

		DirectX11Texture::~DirectX11Texture() {
		}

		DirectX11TextureBinding::DirectX11TextureBinding(TextureBindingCreateInfo ci) {
			texture_ = (DirectX11Texture*)ci.textures->texture;
		}

		void DirectX11TextureBinding::bind() {
			auto device_context = DirectX11GraphicsWrapper::get().device_context_;

			ID3D11ShaderResourceView* srv = texture_->getSrv();
			ID3D11SamplerState* sampler = texture_->getSampler();

			device_context->PSSetShaderResources(0, 1, &srv);
			device_context->PSSetSamplers(0, 1, &sampler);
		}
		
		DirectX11TextureBinding::~DirectX11TextureBinding() {
		}
	};
};
