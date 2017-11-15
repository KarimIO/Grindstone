#include "DxIndexBuffer.hpp"

DxIndexBuffer::DxIndexBuffer(ID3D11Device *device, ID3D11DeviceContext *_deviceContext, IndexBufferCreateInfo createInfo) {
	deviceContext = _deviceContext;
	size = createInfo.size;

	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = createInfo.size;
	vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = createInfo.content;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_indexBuffer))) {
		return;
	}

}

DxIndexBuffer::~DxIndexBuffer() {
	if (m_indexBuffer) {
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}
}

void DxIndexBuffer::Bind() {
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
}
