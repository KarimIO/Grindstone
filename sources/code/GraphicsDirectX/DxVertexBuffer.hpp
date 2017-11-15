#ifndef _DX_VERTEX_BUFFER_H
#define _DX_VERTEX_BUFFER_H

#include <stdint.h>
#include "../GraphicsCommon/VertexBuffer.hpp"
#include <d3d11.h>


class DxVertexBuffer : public VertexBuffer {
private:
	ID3D11DeviceContext *deviceContext;
	uint32_t size;
	uint32_t stride;

	ID3D11Buffer *m_vertexBuffer;
public:
	DxVertexBuffer(ID3D11Device *device, ID3D11DeviceContext *_deviceContext, VertexBufferCreateInfo createInfo);
	~DxVertexBuffer();

	void Bind();
};

#endif