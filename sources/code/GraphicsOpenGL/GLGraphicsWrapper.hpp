#pragma once
#define NOMINMAX
#include <GL/gl3w.h>
#include <vector>

#include "GLRenderPass.hpp"
#include "GLGraphicsPipeline.hpp"
#include "GLFramebuffer.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLUniformBuffer.hpp"
#include "GLTexture.hpp"
#include "GLDepthTarget.hpp"
#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"

#ifdef __linux__
#include <GL/glx.h>
#endif

namespace Grindstone {
	namespace GraphicsAPI {
		GLenum GetGeomType(GeometryType geom_type);

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
			~GLGraphicsWrapper();
			void Clear(int mask);
			void CreateDefaultStructures();

			virtual void DeleteRenderTarget(RenderTarget * ptr) override;
			virtual void DeleteDepthTarget(DepthTarget * ptr) override;
			virtual void DeleteFramebuffer(Framebuffer *ptr) override;
			virtual void DeleteVertexBuffer(VertexBuffer *ptr) override;
			virtual void DeleteIndexBuffer(IndexBuffer *ptr) override;
			virtual void DeleteUniformBuffer(UniformBuffer * ptr) override;
			virtual void DeleteUniformBufferBinding(UniformBufferBinding * ptr) override;
			virtual void DeleteGraphicsPipeline(GraphicsPipeline *ptr) override;
			virtual void DeleteRenderPass(RenderPass *ptr) override;
			virtual void DeleteTexture(Texture *ptr) override;
			virtual void DeleteTextureBinding(TextureBinding *ptr) override;
			virtual void DeleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void DeleteCommandBuffer(CommandBuffer * ptr) override;
			virtual void DeleteVertexArrayObject(VertexArrayObject *ptr) override;

			void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

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
			RenderTarget *CreateRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc, bool cube = false);
			DepthTarget *CreateDepthTarget(DepthTargetCreateInfo rt);
			void CopyToDepthBuffer(DepthTarget *p);

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
			void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount);
			void DrawImmediateVertices(uint32_t base, uint32_t count);
			void SetImmediateBlending(BlendMode);
			void EnableDepth(bool state);
			virtual void BindDefaultFramebuffer(bool depth);
			void SetColorMask(ColorMask mask);

			ColorFormat GetDeviceColorFormat();

			void SwapBuffer();
		};

		extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
			GRAPHICS_EXPORT void deleteGraphics(void *ptr);
		}
	}
}