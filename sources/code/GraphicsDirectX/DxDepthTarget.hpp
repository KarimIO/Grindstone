#ifndef _DX_DEPTH_TARGET_H
#define _DX_DEPTH_TARGET_H

#include <stdint.h>
#include "../GraphicsCommon/DepthTarget.hpp"
#include <d3d11.h>

class DxDepthTarget : public DepthTarget {
public:
	DxDepthTarget(ID3D11Device *device, ID3D11DeviceContext *deviceContext, DepthTargetCreateInfo ci);

	void clear();

	ID3D11Texture2D* getTexture();
	ID3D11DepthStencilView* getTextureView();
	ID3D11SamplerState* getSamplerState();
	ID3D11ShaderResourceView* getSRV();

    virtual ~DxDepthTarget();
private:
	ID3D11Device * device_;
	ID3D11DeviceContext *device_context_;

	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11SamplerState* m_depthSamplerState;
	ID3D11ShaderResourceView* m_depthShaderResourceView;
};

#endif