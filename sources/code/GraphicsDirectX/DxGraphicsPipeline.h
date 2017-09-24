#pragma once

#include <d3d11.h>
#include "../GraphicsCommon/GraphicsPipeline.h"

class DxGraphicsPipeline : public GraphicsPipeline {
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* m_layout;

	bool createShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);
public:
	DxGraphicsPipeline(ID3D11Device* device, ID3D11DeviceContext* deviceContext, GraphicsPipelineCreateInfo createInfo);
	~DxGraphicsPipeline();

	void Bind();
};