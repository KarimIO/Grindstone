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

#ifdef __APPLE__
#define GLFW_WINDOW
#endif

#if defined(GLFW_WINDOW)
#include <GLFW/glfw3.h>
#endif

class InputInterface;

namespace Grindstone {
	namespace GraphicsAPI {

		struct InstanceCreateInfo {
			InputInterface *inputInterface;
			uint32_t width;
			uint32_t height;
			const char *title;
			bool debug;
			bool vsync;
		};

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
			InputInterface *input_;
			const char *title_;
			uint32_t width_;
			uint32_t height_;
			bool debug_;
			bool vsync_;

		public:
			enum class WindowClosingState {
				NotClosing = 0,
				Closing,
				Closed
			} closing_state_;

			virtual void Close();
			virtual void PrepareShutdown();

			// Common windowing functions
			virtual void HandleEvents();
			virtual void SetCursorShown(bool);
			virtual void ResetCursor();
			virtual void SetCursor(int x, int y);
			virtual void GetCursor(int &x, int &y);
			virtual void setFocus();
			virtual void setTitle(const char *title);
			virtual char *getSavePath(const char *filter, const char *default_ext);
			virtual char *getLoadPath(const char *filter, const char *default_ext);
			// Interfaces
		public:
			virtual ~GraphicsWrapper() {};
			virtual void getSwapChainRenderTargets(RenderTarget **&rts, uint32_t &rt_count) = 0;
			virtual void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) = 0;

			virtual void CreateDefaultStructures() = 0;

			virtual void Clear(ClearMode mask) = 0;
			virtual void DeleteRenderTarget(RenderTarget * ptr) = 0;
			virtual void DeleteDepthTarget(DepthTarget * ptr) = 0;
			virtual void DeleteFramebuffer(Framebuffer *ptr) = 0;
			virtual void DeleteVertexBuffer(VertexBuffer *ptr) = 0;
			virtual void DeleteIndexBuffer(IndexBuffer *ptr) = 0;
			virtual void DeleteUniformBuffer(UniformBuffer * ptr) = 0;
			virtual void DeleteUniformBufferBinding(UniformBufferBinding * ptr) = 0;
			virtual void DeleteGraphicsPipeline(GraphicsPipeline *ptr) = 0;
			virtual void DeleteRenderPass(RenderPass *ptr) = 0;
			virtual void DeleteTexture(Texture *ptr) = 0;
			virtual void DeleteTextureBinding(TextureBinding *ptr) = 0;
			virtual void DeleteTextureBindingLayout(TextureBindingLayout *ptr) = 0;
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
			virtual RenderTarget *CreateRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc, bool cube = false) = 0;
			virtual DepthTarget *CreateDepthTarget(DepthTargetCreateInfo rt) = 0;
			virtual void CopyToDepthBuffer(DepthTarget *p) = 0;

			virtual const bool shouldUseImmediateMode() = 0;
			virtual const bool supportsCommandBuffers() = 0;
			virtual const bool supportsTesselation() = 0;
			virtual const bool supportsGeometryShader() = 0;
			virtual const bool supportsComputeShader() = 0;
			virtual const bool supportsMultiDrawIndirect() = 0;

			virtual void BindDefaultFramebuffer(bool depth) = 0;

			virtual uint32_t GetImageIndex() = 0;

			virtual void WaitUntilIdle() = 0;
			virtual void DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) = 0;

			virtual void BindTextureBinding(TextureBinding *) = 0;
			virtual void BindVertexArrayObject(VertexArrayObject *) = 0;
			virtual	void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) = 0;
			virtual void DrawImmediateVertices(uint32_t base, uint32_t count) = 0;
			virtual void SetImmediateBlending(BlendMode) = 0;
			virtual void EnableDepth(bool state) = 0;
			virtual void SetColorMask(ColorMask mask) = 0;

			virtual ColorFormat GetDeviceColorFormat() = 0;

			virtual void SwapBuffer() = 0;
		};
	}
}