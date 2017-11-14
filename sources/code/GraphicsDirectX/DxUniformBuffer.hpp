#ifndef _DX_UNIFORM_BUFFER_H
#define _DX_UNIFORM_BUFFER_H

#include "../GraphicsCommon/UniformBuffer.h"
#include <d3d11.h>

class DxUniformBufferBinding : public UniformBufferBinding {
	UINT binding;
	uint32_t stages;
public:
	DxUniformBufferBinding(UniformBufferBindingCreateInfo);
	UINT GetBinding();
	uint32_t GetStages();
};

class DxUniformBuffer : public UniformBuffer {
private:
	uint32_t size;
	ID3D11Buffer *m_buffer;
	ID3D11DeviceContext* deviceContext;
	DxUniformBufferBinding *binding;
public:
	DxUniformBuffer(ID3D11Device* device, ID3D11DeviceContext* _deviceContext, UniformBufferCreateInfo ci);
	void Bind();
	~DxUniformBuffer();

	void UpdateUniformBuffer(void * content);
};

#endif