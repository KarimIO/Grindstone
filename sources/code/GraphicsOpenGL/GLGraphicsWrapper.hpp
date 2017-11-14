#pragma once
#define NOMINMAX
#include <GL/gl3w.h>
#include <vector>

#include "GLRenderPass.h"
#include "GLGraphicsPipeline.h"
#include "GLFramebuffer.h"
#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"
#include "GLUniformBuffer.h"
#include "GLTexture.h"
#include "../GraphicsCommon/GraphicsWrapper.h"
#include "../GraphicsCommon/DLLDefs.h"

#ifdef __linux__
#include <GL/glx.h>
#endif

class GRAPHICS_EXPORT_CLASS GLGraphicsWrapper : public GraphicsWrapper {
private:
	bool debug;
	bool vsync;

#if defined(GLFW_WINDOW)
#elif defined(_WIN32)
	HGLRC	hRC;
	HDC		hDC;
	bool InitializeWindowContext();
#else
	GLXContext context;
	void CleanX11();
	bool InitializeWindowContext();
#endif

public:
	GLGraphicsWrapper(InstanceCreateInfo createInfo);
	void Clear();
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
	CommandBuffer *CreateCommandBuffer(CommandBufferCreateInfo gp);
	VertexArrayObject *CreateVertexArrayObject(VertexArrayObjectCreateInfo gp);
	VertexBuffer *CreateVertexBuffer(VertexBufferCreateInfo ci);
	IndexBuffer *CreateIndexBuffer(IndexBufferCreateInfo ci);
	UniformBuffer *CreateUniformBuffer(UniformBufferCreateInfo ci);
	UniformBufferBinding *CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci);
	Texture *CreateCubemap(CubemapCreateInfo createInfo);
	Texture *CreateTexture(TextureCreateInfo createInfo);
	TextureBinding *CreateTextureBinding(TextureBindingCreateInfo createInfo);
	TextureBindingLayout *CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo);

	uint32_t GetImageIndex();

	bool SupportsCommandBuffers();
	bool SupportsTesselation();
	bool SupportsGeometryShader();
	bool SupportsComputeShader();
	bool SupportsMultiDrawIndirect();

	void WaitUntilIdle();
	void DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount);
	
	void BindTextureBinding(TextureBinding *);
	void BindVertexArrayObject(VertexArrayObject *);
	void DrawImmediateIndexed(bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount);
	void DrawImmediateVertices(uint32_t base, uint32_t count);
	void SetImmediateBlending(bool);
	void BindDefaultFramebuffer();

	ColorFormat GetDeviceColorFormat();

	void SwapBuffer();
};

extern "C" GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
extern "C" GRAPHICS_EXPORT void deleteGraphics(void *ptr);