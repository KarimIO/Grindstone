#ifndef _GL_TEXTURE_H
#define _GL_TEXTURE_H

#include "../GraphicsCommon/Texture.hpp"
#include <vector>
#include <d3d11.h>

DXGI_FORMAT TranslateFormat(ColorFormat inFormat, unsigned int &pixelSize);

class DxTexture : public Texture {
	ID3D11SamplerState* m_sampleState;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
	bool isCubemap;
public:
	DxTexture(ID3D11Device *m_device, ID3D11DeviceContext *m_deviceContext, TextureCreateInfo createInfo);
	DxTexture(ID3D11Device *m_device, ID3D11DeviceContext *m_deviceContext, CubemapCreateInfo createInfo);
	ID3D11ShaderResourceView *GetTextureView();
	ID3D11SamplerState *GetSampler();
	~DxTexture();
};

class DxTextureBindingLayout;

class DxTextureBinding : public TextureBinding {
	ID3D11DeviceContext *m_deviceContext;
	std::vector<DxTexture *> m_textures;
	DxTextureBindingLayout *layout_;
	unsigned int first_address_;
public:
	DxTextureBinding(ID3D11DeviceContext *deviceContext, TextureBindingCreateInfo ci);
	void Bind();
};

class DxTextureBindingLayout : public TextureBindingLayout {
public:
	TextureSubBinding *subbindings;
	uint32_t stages;
	uint32_t subbindingCount;
	DxTextureBindingLayout(TextureBindingLayoutCreateInfo);
};

#endif