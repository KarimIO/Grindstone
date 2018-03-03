#ifndef _DX_RENDER_TARGET_H
#define _DX_RENDER_TARGET_H

#include <stdint.h>
#include "../GraphicsCommon/RenderTarget.hpp"
#include <d3d11.h>

class DxRenderTarget : public RenderTarget {
public:
	DxRenderTarget(ID3D11Device *device, ID3D11DeviceContext *deviceContext, RenderTargetCreateInfo *cis, uint32_t count);

	void clear();

	uint32_t getSize();
	ID3D11RenderTargetView** getTextureViews();
	ID3D11SamplerState** getSamplerStates();
	ID3D11ShaderResourceView** getSRVs();
	virtual unsigned char *RenderScreen(unsigned int i, unsigned int resx, unsigned int resy);

    virtual ~DxRenderTarget();
private:
	ID3D11Device * device_;
	ID3D11DeviceContext *device_context_;
	uint32_t m_numRenderTargets;
	ID3D11Texture2D** m_renderTargetTexture;
	ID3D11RenderTargetView** m_renderTargetView;
	ID3D11SamplerState** m_sampleStates;
	ID3D11ShaderResourceView** m_shaderResourceView;

	uint32_t width_, height_;
};

#endif