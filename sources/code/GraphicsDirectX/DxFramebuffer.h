#ifndef _DX_FRAMEBUFFER_H
#define _DX_FRAMEBUFFER_H

#include "../GraphicsCommon/Framebuffer.h"
#include "../GraphicsCommon/DLLDefs.h"
#include <d3d11.h>

class DxFramebuffer : public Framebuffer {
	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;

	uint32_t m_numRenderTargets;
	ID3D11Texture2D** m_renderTargetTexture;
	ID3D11RenderTargetView** m_renderTargetView;
	ID3D11ShaderResourceView** m_shaderResourceView;
	ID3D11SamplerState** m_sampleStates;

	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11SamplerState* m_depthSamplerState;
	ID3D11ShaderResourceView* m_depthShaderResourceView;

	uint32_t width, height;
public:
	DxFramebuffer(ID3D11Device *device, ID3D11DeviceContext *deviceContext, FramebufferCreateInfo);
	~DxFramebuffer();
	void Clear();
	void Blit(int i, int x, int y, int w, int h);
	void BindWrite();
	void BindRead();
	void BindTextures();
	void Unbind();
};

#endif