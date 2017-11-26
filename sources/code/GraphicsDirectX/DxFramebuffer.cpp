#include "DxFramebuffer.hpp"
#include <iostream>

DxFramebuffer::DxFramebuffer(ID3D11Device *device, ID3D11DeviceContext *deviceContext, FramebufferCreateInfo ci) {
	m_device = device;
	m_deviceContext = deviceContext;
}

DxFramebuffer::~DxFramebuffer() {
}

void DxFramebuffer::Clear() {
	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (uint32_t i = 0; i < m_numRenderTargets; i++)
		m_deviceContext->ClearRenderTargetView(m_renderTargetView[i], color);

	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DxFramebuffer::CopyFrom(Framebuffer *) {
}

void DxFramebuffer::Bind() {
}

void DxFramebuffer::BindWrite() {
	std::vector<ID3D11ShaderResourceView*>		textures;

	textures.reserve(m_numRenderTargets + 1);
	for (size_t i = 0; i < m_numRenderTargets + 1; i++) {
		textures.push_back(NULL);
	}
	m_deviceContext->PSSetShaderResources(0, textures.size(), textures.data());
	m_deviceContext->OMSetRenderTargets(m_numRenderTargets, m_renderTargetView, m_depthStencilView);
}

void DxFramebuffer::BindRead() {
}

void DxFramebuffer::Unbind()
{
}
