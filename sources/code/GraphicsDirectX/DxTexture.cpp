#include "DxTexture.hpp"
#include <iostream>

DxTexture::DxTexture(ID3D11Device *device, ID3D11DeviceContext *deviceContext, TextureCreateInfo createInfo) {
	
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
	if (FAILED(device->CreateSamplerState(&samplerDesc, &m_sampleState))) {
		std::cout << "Sampler View failed to be created!\n";
		return;
	}

	unsigned int pixelSize = 0;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = createInfo.width;
	textureDesc.Height = createInfo.height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TranslateFormat(createInfo.format, pixelSize);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the empty texture.
	HRESULT hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult)) {
		std::cout << "Texture failed to be created!\n";
		return;
	}

	// Set the row pitch of the targa image data.
	unsigned int rowPitch = (createInfo.width) * 4 * sizeof(unsigned char);

	deviceContext->UpdateSubresource(m_texture, 0, NULL, createInfo.data, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult)) {
		std::cout << "Texture View failed to be created!\n";
		return;
	}

	// Generate mipmaps for this texture.
	deviceContext->GenerateMips(m_textureView);
}

DxTexture::DxTexture(ID3D11Device *device, ID3D11DeviceContext *deviceContext, CubemapCreateInfo createInfo) {
	
}

ID3D11ShaderResourceView *DxTexture::GetTextureView() {
	return m_textureView;
}

ID3D11SamplerState * DxTexture::GetSampler() {
	return m_sampleState;
}

DxTexture::~DxTexture() {
	if (m_textureView) {
		m_textureView->Release();
		m_textureView = 0;
	}

	if (m_texture) {
		m_texture->Release();
		m_texture = 0;
	}

	if (m_sampleState) {
		m_sampleState->Release();
		m_sampleState = 0;
	}
}

DXGI_FORMAT TranslateFormat(ColorFormat inFormat, unsigned int &pixelSize) {
	switch (inFormat) {
	case FORMAT_COLOR_R8:
		pixelSize = sizeof(unsigned char);
		return DXGI_FORMAT_R8_UNORM;
	case FORMAT_COLOR_R8G8:
		pixelSize = 2 * sizeof(unsigned char);
		return DXGI_FORMAT_R8G8_UNORM;
	case FORMAT_COLOR_R8G8B8:
		std::cout << "FORMAT_COLOR_R8G8B8 unsupported.\n";
		pixelSize = 3 * sizeof(unsigned char);
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case FORMAT_COLOR_R8G8B8A8:
		pixelSize = 4 * sizeof(unsigned char);
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

DxTextureBinding::DxTextureBinding(ID3D11DeviceContext *deviceContext, TextureBindingCreateInfo ci) {
	m_deviceContext = deviceContext;

	m_textures.reserve(ci.textureCount);
	for (uint32_t i = 0; i < ci.textureCount; i++) {
		m_textures.push_back((DxTexture *)ci.textures[i].texture);
	}
}

void DxTextureBinding::Bind() {
	std::vector<ID3D11ShaderResourceView*>		textures;
	std::vector<ID3D11SamplerState*>	samplers;

	textures.reserve(m_textures.size());
	samplers.reserve(m_textures.size());
	size_t count = m_textures.size();
	for (size_t i = 0; i < count; i++) {
		textures.push_back(m_textures[i]->GetTextureView());
		samplers.push_back(m_textures[i]->GetSampler());
	}

	ID3D11ShaderResourceView *const *texData = (ID3D11ShaderResourceView * const *)textures.data();
	ID3D11SamplerState **samplersData = samplers.data();
	m_deviceContext->PSSetSamplers(0, count, samplersData);
	m_deviceContext->PSSetShaderResources(0, count, texData);
}

DxTextureBindingLayout::DxTextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
	
}
