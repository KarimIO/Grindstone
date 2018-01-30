#include "DxGraphicsPipeline.hpp"

bool DxGraphicsPipeline::createShaderModule(ShaderStageCreateInfo shaderStageCreateInfo) {
	switch (shaderStageCreateInfo.type) {
	case SHADER_FRAGMENT:
		if (FAILED(device->CreatePixelShader(shaderStageCreateInfo.content, shaderStageCreateInfo.size, NULL, &pixelShader))) {
			return false;
		}
	case SHADER_TESS_EVALUATION:
		return false;
	case SHADER_TESS_CONTROL:
		return false;
	case SHADER_GEOMETRY:
		return false;
	default:
	case SHADER_VERTEX:
		if (FAILED(device->CreateVertexShader(shaderStageCreateInfo.content, shaderStageCreateInfo.size, NULL, &vertexShader))) {
			return false;
		}
	}

	return true;
}

LPCSTR TranslateUsage(AttributeUsage usage) {
	switch (usage) {
	default:
	case ATTRIB_POSITION:
		return "POSITION";
	case ATTRIB_TEXCOORD0:
		return "TEXCOORD";
	case ATTRIB_TEXCOORD1:
		return "TEXCOORD";
	case ATTRIB_NORMAL:
		return "NORMAL";
	case ATTRIB_TANGENT:
		return "TANGENT";
	}
}

DXGI_FORMAT TranslateFormat(VertexFormat format) {
	if (format == VERTEX_R32_G32)
		return DXGI_FORMAT_R32G32_FLOAT;
	else
		return DXGI_FORMAT_R32G32B32_FLOAT;
}

DxGraphicsPipeline::DxGraphicsPipeline(ID3D11Device *_device, ID3D11DeviceContext *_deviceContext, GraphicsPipelineCreateInfo createInfo) {
	device = _device;
	deviceContext = _deviceContext;
	
	for (uint32_t i = 0; i < createInfo.shaderStageCreateInfoCount; i++) {
		createShaderModule(createInfo.shaderStageCreateInfos[i]);
	}

	std::vector <D3D11_INPUT_ELEMENT_DESC> polygonLayout;
	polygonLayout.resize(createInfo.attributesCount);
	for (uint32_t i = 0; i < createInfo.attributesCount; i++) {
		polygonLayout[i].SemanticName = TranslateUsage(createInfo.attributes[i].usage);
		polygonLayout[i].SemanticIndex = 0;
		polygonLayout[i].Format = TranslateFormat(createInfo.attributes[i].format);
		polygonLayout[i].InputSlot = 0;
		polygonLayout[i].AlignedByteOffset = (i == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[i].InstanceDataStepRate = 0;
	}

	HRESULT res = device->CreateInputLayout(polygonLayout.data(), polygonLayout.size(), createInfo.shaderStageCreateInfos[0].content,
		createInfo.shaderStageCreateInfos[0].size, &m_layout);
	if (FAILED(res)) {
		std::cout << "Failed to Create DX Input Layout\n";
		return;
	}

	viewport.Width = (float)createInfo.width;
	viewport.Height = (float)createInfo.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
}

DxGraphicsPipeline::~DxGraphicsPipeline() {
	if (m_layout) {
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (pixelShader) {
		pixelShader->Release();
		pixelShader = 0;
	}

	// Release the vertex shader.
	if (vertexShader) {
		vertexShader->Release();
		vertexShader = 0;
	}
}

void DxGraphicsPipeline::Bind() {
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the viewport.
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	deviceContext->RSSetViewports(1, &viewport);
}
