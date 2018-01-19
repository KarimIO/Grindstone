#include "DxFramebuffer.hpp"
#include <iostream>

DxFramebuffer::DxFramebuffer(ID3D11Device *device, ID3D11DeviceContext *deviceContext, FramebufferCreateInfo ci) : m_device(device), m_deviceContext(deviceContext), depth_target_((DxDepthTarget *)ci.depth_target) {

	render_target_lists_.resize(ci.num_render_target_lists);
	for (int i = 0; i < ci.num_render_target_lists; i++) {
		render_target_lists_[i] = (DxRenderTarget *)ci.render_target_lists[i];
	}

	int s = 0;
	int k = 0;
	DxRenderTarget **rt_list = (DxRenderTarget **)(ci.render_target_lists);
	for (int i = 0; i < ci.num_render_target_lists; i++) {
		s += rt_list[i]->getSize();
		srvs_.resize(s);
		render_targets_.resize(s);
		samplers_.resize(s);

		ID3D11RenderTargetView ** rts = rt_list[i]->getTextureViews();
		ID3D11ShaderResourceView **srvs = rt_list[i]->getSRVs();
		ID3D11SamplerState **samplers = rt_list[i]->getSamplerStates();

		for (int j = 0; j < rt_list[i]->getSize(); j++) {
			render_targets_[k] = rts[j];
			samplers_[k] = samplers[j];
			srvs_[k] = srvs[j];
			++k;
		}
	}

	samplers_.push_back(samplers_[0]);
	//samplers_.push_back(depth_target_->getSamplerState());
	srvs_.push_back(depth_target_->getSRV());
}

DxFramebuffer::~DxFramebuffer() {
}

void DxFramebuffer::Clear() {

	for (uint32_t i = 0; i < render_target_lists_.size(); i++)
		render_target_lists_.clear();

	if (depth_target_ != nullptr)
		depth_target_->clear();
}

void DxFramebuffer::CopyFrom(Framebuffer *) {
}

void DxFramebuffer::Bind() {
	BindWrite();
	//BindRead();
}

void DxFramebuffer::BindWrite() {
	std::vector<ID3D11ShaderResourceView*>		textures;

	textures.reserve(srvs_.size() + 1);
	for (size_t i = 0; i < srvs_.size() + 1; i++) {
		textures.push_back(NULL);
	}

	m_deviceContext->PSSetShaderResources(0, textures.size(), textures.data());
	m_deviceContext->OMSetRenderTargets(render_targets_.size(), render_targets_.data(), depth_target_->getTextureView());
}

void DxFramebuffer::BindRead() {
	/*std::vector<ID3D11ShaderResourceView*>	textures;
	std::vector<ID3D11SamplerState*>		samplers;

	textures.reserve(m_numRenderTargets + 1);
	samplers.reserve(m_numRenderTargets + 1);
	for (size_t i = 0; i < m_numRenderTargets; i++) {
		textures.push_back(m_shaderResourceView[i]);
		samplers.push_back(m_sampleStates[i]);
	}
	textures.push_back(m_depthShaderResourceView);
	samplers.push_back(m_sampleStates[0]);*/

	m_deviceContext->PSSetShaderResources(0, srvs_.size(), srvs_.data());
	m_deviceContext->PSSetSamplers(0, samplers_.size(), samplers_.data());

}

void DxFramebuffer::Unbind()
{
}
