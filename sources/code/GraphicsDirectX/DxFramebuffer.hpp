#ifndef _DX_FRAMEBUFFER_H
#define _DX_FRAMEBUFFER_H

#include "../GraphicsCommon/Framebuffer.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"
#include <d3d11.h>

class DxFramebuffer : public Framebuffer {
	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;

	uint32_t width, height;
public:
	DxFramebuffer(ID3D11Device *device, ID3D11DeviceContext *deviceContext, FramebufferCreateInfo);
	~DxFramebuffer();
	void Clear();
	void CopyFrom(Framebuffer *);
	void Bind();
	void BindWrite();
	void BindRead();
	void Unbind();
};

#endif