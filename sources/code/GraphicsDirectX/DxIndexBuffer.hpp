#ifndef _DX_INDEX_BUFFER_H
#define _DX_INDEX_BUFFER_H

#include <stdint.h>
#include "../GraphicsCommon/IndexBuffer.hpp"
#include <d3d11.h>


class DxIndexBuffer : public IndexBuffer {
private:
	ID3D11DeviceContext *deviceContext;
	uint32_t size;

	ID3D11Buffer *m_indexBuffer;
public:
	DxIndexBuffer(ID3D11Device *device, ID3D11DeviceContext *_deviceContext, IndexBufferCreateInfo createInfo);
	~DxIndexBuffer();

	void Bind();
};

#endif