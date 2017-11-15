#include "DxUniformBuffer.hpp"
#include <iostream>

DxUniformBuffer::DxUniformBuffer(ID3D11Device* device, ID3D11DeviceContext* _deviceContext, UniformBufferCreateInfo ci) {
	size = ci.size;
	deviceContext = _deviceContext;
	binding = (DxUniformBufferBinding *)ci.binding;

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = ci.size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	if (FAILED(device->CreateBuffer(&bufferDesc, NULL, &m_buffer))) {
		std::cerr << "Failed to Create DX Uniform Buffer\n";
		return;
	}
}

void DxUniformBuffer::Bind() {
	// Check which stages to send to here.
	uint32_t stages = binding->GetStages();
	if ((stages & SHADER_STAGE_VERTEX_BIT) == SHADER_STAGE_VERTEX_BIT)
		deviceContext->VSSetConstantBuffers(binding->GetBinding(), 1, &m_buffer);
	if ((stages & SHADER_STAGE_FRAGMENT_BIT) == SHADER_STAGE_FRAGMENT_BIT)
		deviceContext->PSSetConstantBuffers(binding->GetBinding(), 1, &m_buffer);
	if ((stages & SHADER_STAGE_COMPUTE_BIT) == SHADER_STAGE_COMPUTE_BIT)
		deviceContext->CSSetConstantBuffers(binding->GetBinding(), 1, &m_buffer);
	if ((stages & SHADER_STAGE_TESSELLATION_EVALUATION_BIT) == SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
		deviceContext->DSSetConstantBuffers(binding->GetBinding(), 1, &m_buffer);
	if ((stages & SHADER_STAGE_TESSELLATION_CONTROL_BIT) == SHADER_STAGE_TESSELLATION_CONTROL_BIT)
		deviceContext->HSSetConstantBuffers(binding->GetBinding(), 1, &m_buffer);
	if ((stages & SHADER_STAGE_GEOMETRY_BIT) == SHADER_STAGE_GEOMETRY_BIT)
		deviceContext->GSSetConstantBuffers(binding->GetBinding(), 1, &m_buffer);
}

DxUniformBuffer::~DxUniformBuffer() {
	if (m_buffer) {
		m_buffer->Release();
		m_buffer = 0;
	}

}

void DxUniformBuffer::UpdateUniformBuffer(void * content) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (FAILED(deviceContext->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		return;
	}

	memcpy(mappedResource.pData, content, size);

	// Unlock the constant buffer.
	deviceContext->Unmap(m_buffer, 0);
}

DxUniformBufferBinding::DxUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
	binding = ci.binding;
	stages = ci.stages;
}

UINT DxUniformBufferBinding::GetBinding() {
	return binding;
}

uint32_t DxUniformBufferBinding::GetStages()
{
	return stages;
}
