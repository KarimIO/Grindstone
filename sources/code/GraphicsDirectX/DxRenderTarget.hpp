#ifndef _DX_RENDER_TARGET_H
#define _DX_RENDER_TARGET_H

#include <stdint.h>
#include "../GraphicsCommon/RenderTarget.hpp"
#include <d3d11.h>

class DxRenderTarget : public RenderTarget {
public:
	DxRenderTarget(ID3D11Device *device, ID3D11DeviceContext *deviceContext, RenderTargetCreateInfo *cis, uint32_t count);
    uint32_t getHandle();
    uint32_t getHandle(uint32_t i);
    uint32_t getNumRenderTargets();

    virtual void Bind();
    virtual void Bind(uint32_t i);
    virtual ~DxRenderTarget();
private:
	uint32_t m_numRenderTargets;
	ID3D11Texture2D** m_renderTargetTexture;
	ID3D11RenderTargetView** m_renderTargetView;
	ID3D11SamplerState** m_sampleStates;
	ID3D11ShaderResourceView** m_shaderResourceView;
};

#endif