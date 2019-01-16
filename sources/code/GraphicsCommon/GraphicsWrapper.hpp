#pragma once

#include "IndexBuffer.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "VertexBuffer.hpp"
#include "UniformBuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "CommandBuffer.hpp"
#include "VertexArrayObject.hpp"
#include "DepthTarget.hpp"

#ifdef _WIN32
	#include <Windows.h>
	#include <Windowsx.h>
#else
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/keysymdef.h>
#endif

class InputInterface;

struct InstanceCreateInfo {
	InputInterface *inputInterface;
	uint32_t width;
	uint32_t height;
	const char *title;
	bool debug;
	bool vsync;
};

#ifdef __APPLE__
#define GLFW_WINDOW
#endif

#if defined(GLFW_WINDOW)
#include <GLFW/glfw3.h>
#endif

enum BlendMode {
	BLEND_NONE = 0,
	BLEND_ADDITIVE,
	BLEND_ADD_ALPHA,
};

enum GrindstoneGeometryType {
	GEOMETRY_POINTS,
	GEOMETRY_LINES,
	GEOMETRY_LINE_STRIP,
	GEOMETRY_LINE_LOOP,
	GEOMETRY_TRIANGLE_STRIP,
	GEOMETRY_TRIANGLE_FAN,
	GEOMETRY_TRIANGLES,
	GEOMETRY_LINES_ADJACENCY,
	GEOMETRY_TRIANGLES_ADJACENCY,
	GEOMETRY_TRIANGLE_STRIP_ADJACENCY,
	GEOMETRY_PATCHES
};

#define COLOR_MASK_RED 0x1
#define COLOR_MASK_GREEN 0x2
#define COLOR_MASK_BLUE 0x4
#define COLOR_MASK_ALPHA 0x8
#define COLOR_MASK_RG COLOR_MASK_RED | COLOR_MASK_GREEN
#define COLOR_MASK_RGB COLOR_MASK_RG | COLOR_MASK_BLUE
#define COLOR_MASK_RGBA COLOR_MASK_RGB | COLOR_MASK_ALPHA

class GraphicsWrapper {
public:

#if defined(GLFW_WINDOW)
	GLFWwindow* window;
	bool InitializeWindowContext();
#elif defined(_WIN32)
	static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT	CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND	window_handle;

	bool InitializeWin32Window();
#else
	Display *xDisplay;
	Window xWindow;
#endif
	InputInterface *input;
	const char *title;
	uint32_t width;
	uint32_t height;
public:
	virtual ~GraphicsWrapper() {};

	virtual void HandleEvents();
	virtual void SetCursorShown(bool);
	virtual void ResetCursor();
	virtual void SetCursor(int x, int y);
	virtual void GetCursor(int &x, int &y);
	virtual void setFocus();

	virtual void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) = 0;

	virtual void CreateDefaultStructures() = 0;

	virtual void Clear(int mask) = 0;
	virtual void DeleteFramebuffer(Framebuffer *ptr) = 0;
	virtual void DeleteVertexBuffer(VertexBuffer *ptr) = 0;
	virtual void DeleteIndexBuffer(IndexBuffer *ptr) = 0;
	virtual void DeleteUniformBuffer(UniformBuffer * ptr) = 0;
	virtual void DeleteUniformBufferBinding(UniformBufferBinding * ptr) = 0;
	virtual void DeleteGraphicsPipeline(GraphicsPipeline *ptr) = 0;
	virtual void DeleteRenderPass(RenderPass *ptr) = 0;
	virtual void DeleteTexture(Texture *ptr) = 0;
	virtual void DeleteCommandBuffer(CommandBuffer *ptr) = 0;
	virtual void DeleteVertexArrayObject(VertexArrayObject *ptr) = 0;

	virtual Framebuffer *CreateFramebuffer(FramebufferCreateInfo ci) = 0;
	virtual RenderPass *CreateRenderPass(RenderPassCreateInfo ci) = 0;
	virtual GraphicsPipeline *CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci) = 0;
	virtual void CreateDefaultFramebuffers(DefaultFramebufferCreateInfo ci, Framebuffer **&framebuffers, uint32_t &framebufferCount) = 0;
	virtual CommandBuffer *CreateCommandBuffer(CommandBufferCreateInfo ci) = 0;
	virtual VertexArrayObject *CreateVertexArrayObject(VertexArrayObjectCreateInfo ci) = 0;
	virtual VertexBuffer *CreateVertexBuffer(VertexBufferCreateInfo ci) = 0;
	virtual IndexBuffer *CreateIndexBuffer(IndexBufferCreateInfo ci) = 0;
	virtual UniformBuffer *CreateUniformBuffer(UniformBufferCreateInfo ci) = 0;
	virtual UniformBufferBinding *CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci) = 0;
	virtual Texture *CreateCubemap(CubemapCreateInfo createInfo) = 0;
	virtual Texture *CreateTexture(TextureCreateInfo createInfo) = 0;
	virtual TextureBinding *CreateTextureBinding(TextureBindingCreateInfo ci) = 0;
	virtual TextureBindingLayout *CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) = 0;
	virtual RenderTarget *CreateRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc) = 0;
	virtual DepthTarget *CreateDepthTarget(DepthTargetCreateInfo rt) = 0;
	virtual void CopyToDepthBuffer(DepthTarget *p) = 0;

	virtual bool SupportsCommandBuffers() = 0;
	virtual bool SupportsTesselation() = 0;
	virtual bool SupportsGeometryShader() = 0;
	virtual bool SupportsComputeShader() = 0;
	virtual bool SupportsMultiDrawIndirect() = 0;
	virtual void BindDefaultFramebuffer(bool depth) = 0;

	virtual uint32_t GetImageIndex() = 0;

	virtual void WaitUntilIdle() = 0;
	virtual void DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) = 0;

	virtual void BindTextureBinding(TextureBinding *) = 0;
	virtual void BindVertexArrayObject(VertexArrayObject *) = 0;
	virtual	void DrawImmediateIndexed(GrindstoneGeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) = 0;
	virtual void DrawImmediateVertices(uint32_t base, uint32_t count) = 0;
	virtual void SetImmediateBlending(BlendMode) = 0;
	virtual void EnableDepth(bool state) = 0;
	virtual void SetColorMask(uint8_t mask) = 0;

	virtual ColorFormat GetDeviceColorFormat() = 0;

	virtual void SwapBuffer() = 0;
};