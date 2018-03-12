#include "DxRenderTarget.hpp"
#include <iostream>

DxRenderTarget::DxRenderTarget(ID3D11Device *device, ID3D11DeviceContext *device_context, RenderTargetCreateInfo *create_info, uint32_t count) : device_(device), device_context_(device_context) {
	m_numRenderTargets = count;

	width_ = create_info->width;
	height_ = create_info->height;
	m_renderTargetTexture = new ID3D11Texture2D *[m_numRenderTargets];
	m_renderTargetView = new ID3D11RenderTargetView *[m_numRenderTargets];
	m_shaderResourceView = new ID3D11ShaderResourceView *[m_numRenderTargets];
	m_sampleStates = new ID3D11SamplerState *[m_numRenderTargets];
    
    for (int i = 0; i < count; i++) {
		HRESULT result;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


		D3D11_SAMPLER_DESC samplerDesc;
		// Create a texture sampler state description.
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		if (FAILED(device->CreateSamplerState(&samplerDesc, &m_sampleStates[i]))) {
			std::cout << "Sampler View failed to be created!\n";
			return;
		}

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		// Setup the render target texture description.
		textureDesc.Width = create_info[i].width;
		textureDesc.Height = create_info[i].height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		//ci.colorFormats[i];
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
		result = device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture[i]);
		if (FAILED(result)) {
			return;
		}

		// Setup the description of the render target view.
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		result = device->CreateRenderTargetView(m_renderTargetTexture[i], &renderTargetViewDesc, &m_renderTargetView[i]);
		if (FAILED(result)) {
			return;
		}

		// Setup the description of the shader resource view.
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		result = device->CreateShaderResourceView(m_renderTargetTexture[i], &shaderResourceViewDesc, &m_shaderResourceView[i]);
		if (FAILED(result)) {
			return;
		}
    }
}

void DxRenderTarget::clear() {
	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (uint32_t i = 0; i < m_numRenderTargets; i++)
		device_context_->ClearRenderTargetView(m_renderTargetView[i], color);
}

uint32_t DxRenderTarget::getSize() {
	return m_numRenderTargets;
}

ID3D11RenderTargetView **DxRenderTarget::getTextureViews() {
	return m_renderTargetView;
}

ID3D11SamplerState **DxRenderTarget::getSamplerStates() {
	return m_sampleStates;
}

ID3D11ShaderResourceView **DxRenderTarget::getSRVs() {
	return m_shaderResourceView;
}

unsigned char * DxRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) {
	HRESULT hr;

	ID3D11Resource* pSurface = nullptr;
	m_renderTargetView[i]->GetResource(&pSurface);


	unsigned char *p = new unsigned char[width_ * height_ * 4];

	if (pSurface)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = width_;
		desc.Height = height_;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		ID3D11Texture2D* pTexture = nullptr;
		hr = device_->CreateTexture2D(&desc, nullptr, &pTexture);
		if (pTexture)
		{
			device_context_->CopyResource(pTexture, pSurface);
			GUID g;
			UINT size;
			pSurface->GetPrivateData(g, &size, p);


			pTexture->Release();
		}
		pSurface->Release();
	}
	
	return p;
}

DxRenderTarget::~DxRenderTarget() {
	for (uint32_t i = 0; i < m_numRenderTargets; i++) {
		m_renderTargetTexture[i]->Release();
		m_renderTargetView[i]->Release();
		m_shaderResourceView[i]->Release();
	}

	delete[] m_renderTargetTexture;
	delete[] m_renderTargetView;
	delete[] m_shaderResourceView;
}