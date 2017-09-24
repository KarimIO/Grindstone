#include "DxVertexBuffer.h"

DxVertexBuffer::DxVertexBuffer(ID3D11Device *device, ID3D11DeviceContext *_deviceContext, VertexBufferCreateInfo createInfo) {
	deviceContext = _deviceContext;
	size = createInfo.size;
	stride = size / createInfo.count;
	
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = createInfo.size;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = createInfo.content;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer))) {
		return;
	}

}

DxVertexBuffer::~DxVertexBuffer() {
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

void DxVertexBuffer::Bind() {
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
}
