#pragma once
#define NOMINMAX
#include <d3d11.h>
#include <vector>

//#include "DxRenderPass.h"
//#include "DxGraphicsPipeline.h"
//#include "DxFramebuffer.h"
//#include "DxCommandBuffer.h"
//#include "DxVertexBuffer.h"
//#include "DxIndexBuffer.h"
//#include "DxUniformBuffer.h"
//#include "DxTexture.h"
#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"
#include "../GraphicsCommon/VertexArrayObject.hpp"

class GRAPHICS_EXPORT_CLASS DxGraphicsWrapper : public GraphicsWrapper {
private:
	bool vsync;
	bool debug;

	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;
	IDXGISwapChain *m_swapChain;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;

	ID3D11BlendState* m_alphaBlendState;
	ID3D11BlendState* m_addBlendState;
	ID3D11BlendState* m_noBlendState;
public:
	void Clear();
	
	DxGraphicsWrapper(InstanceCreateInfo createInfo);
	void CreateDefaultStructures();
	void Cleanup();

	void DeleteFramebuffer(Framebuffer *ptr);
	void DeleteVertexBuffer(VertexBuffer *ptr);
	void DeleteIndexBuffer(IndexBuffer *ptr);
	void DeleteUniformBuffer(UniformBuffer * ptr);
	void DeleteUniformBufferBinding(UniformBufferBinding * ptr);
	void DeleteGraphicsPipeline(GraphicsPipeline *ptr);
	void DeleteRenderPass(RenderPass *ptr);
	void DeleteTexture(Texture *ptr);
	void DeleteCommandBuffer(CommandBuffer * ptr);
	void DeleteVertexArrayObject(VertexArrayObject *ptr);

	Framebuffer *CreateFramebuffer(FramebufferCreateInfo ci);
	RenderPass *CreateRenderPass(RenderPassCreateInfo ci);
	GraphicsPipeline *CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci);
	void CreateDefaultFramebuffers(DefaultFramebufferCreateInfo ci, Framebuffer **&framebuffers, uint32_t &framebufferCount);
	CommandBuffer *CreateCommandBuffer(CommandBufferCreateInfo ci);
	VertexArrayObject *CreateVertexArrayObject(VertexArrayObjectCreateInfo ci);
	VertexBuffer *CreateVertexBuffer(VertexBufferCreateInfo ci);
	IndexBuffer *CreateIndexBuffer(IndexBufferCreateInfo ci);
	UniformBuffer *CreateUniformBuffer(UniformBufferCreateInfo ci);
	UniformBufferBinding *CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci);
	Texture *CreateCubemap(CubemapCreateInfo createInfo);
	Texture *CreateTexture(TextureCreateInfo createInfo);
	TextureBinding *CreateTextureBinding(TextureBindingCreateInfo createInfo);
	TextureBindingLayout *CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo);

	bool SupportsCommandBuffers();
	bool SupportsTesselation();
	bool SupportsGeometryShader();
	bool SupportsComputeShader();
	bool SupportsMultiDrawIndirect();

	uint32_t GetImageIndex();

	void BindTextureBinding(TextureBinding *);
	void BindVertexArrayObject(VertexArrayObject *);
	
	void WaitUntilIdle();
	void DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount);
	void DrawImmediateIndexed(bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount);
	void DrawImmediateVertices(uint32_t base, uint32_t count);
	void SetImmediateBlending(BlendMode);
	void BindDefaultFramebuffer();

	ColorFormat GetDeviceColorFormat();

	void SwapBuffer();
};

extern "C" GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
extern "C" GRAPHICS_EXPORT void deleteGraphics(void *ptr);