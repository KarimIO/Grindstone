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

namespace Grindstone {
	namespace GraphicsAPI {
		GLenum GetGeomType(GeometryType geom_type);

		class GLGraphicsWrapper : public GraphicsWrapper {
		public:
			virtual bool initialize(GraphicsWrapperCreateInfo createInfo) override;
			~GLGraphicsWrapper();
			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
			virtual void swapBuffers() override;
			virtual void adjustPerspective(float *perspective) override;

			virtual const char* getVendorName() override;
			virtual const char* getAdapterName() override;
			virtual const char* getAPIName() override;
			virtual const char* getAPIVersion() override;

			virtual void getSwapChainRenderTargets(RenderTarget **&rts, uint32_t &rt_count) override;
			virtual void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override;

			virtual void deleteRenderTarget(RenderTarget * ptr) override;
			virtual void deleteDepthTarget(DepthTarget * ptr) override;
			virtual void deleteFramebuffer(Framebuffer *ptr) override;
			virtual void deleteVertexBuffer(VertexBuffer *ptr) override;
			virtual void deleteIndexBuffer(IndexBuffer *ptr) override;
			virtual void deleteUniformBuffer(UniformBuffer * ptr) override;
			virtual void deleteUniformBufferBinding(UniformBufferBinding * ptr) override;
			virtual void deleteGraphicsPipeline(GraphicsPipeline *ptr) override;
			virtual void deleteRenderPass(RenderPass *ptr) override;
			virtual void deleteTexture(Texture *ptr) override;
			virtual void deleteTextureBinding(TextureBinding *ptr) override;
			virtual void deleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void deleteCommandBuffer(CommandBuffer * ptr) override;
			virtual void deleteVertexArrayObject(VertexArrayObject *ptr) override;

			virtual Framebuffer *createFramebuffer(FramebufferCreateInfo ci) override;
			virtual RenderPass *createRenderPass(RenderPassCreateInfo ci) override;
			virtual GraphicsPipeline *createGraphicsPipeline(GraphicsPipelineCreateInfo ci) override;
			virtual CommandBuffer *createCommandBuffer(CommandBufferCreateInfo gp) override;
			virtual VertexArrayObject *createVertexArrayObject(VertexArrayObjectCreateInfo gp) override;
			virtual VertexBuffer *createVertexBuffer(VertexBufferCreateInfo ci) override;
			virtual IndexBuffer *createIndexBuffer(IndexBufferCreateInfo ci) override;
			virtual UniformBuffer *createUniformBuffer(UniformBufferCreateInfo ci) override;
			virtual UniformBufferBinding *createUniformBufferBinding(UniformBufferBindingCreateInfo ci) override;
			virtual Texture *createCubemap(CubemapCreateInfo createInfo) override;
			virtual Texture *createTexture(TextureCreateInfo createInfo) override;
			virtual TextureBinding *createTextureBinding(TextureBindingCreateInfo createInfo) override;
			virtual TextureBindingLayout *createTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) override;
			virtual RenderTarget *createRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget *createDepthTarget(DepthTargetCreateInfo rt) override;

			virtual void copyToDepthBuffer(DepthTarget *p) override;

			virtual uint32_t getImageIndex() override;

			virtual const bool shouldUseImmediateMode() override;
			virtual const bool supportsCommandBuffers() override;
			virtual const bool supportsTesselation() override;
			virtual const bool supportsGeometryShader() override;
			virtual const bool supportsComputeShader() override;
			virtual const bool supportsMultiDrawIndirect() override;

			virtual void waitUntilIdle() override;
			virtual void drawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) override;

			virtual void bindTextureBinding(TextureBinding *) override;
			virtual void bindVertexArrayObject(VertexArrayObject *) override;
			virtual void drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
			virtual void drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
			virtual void setImmediateBlending(BlendMode) override;
			virtual void enableDepth(bool state) override;
			virtual void bindDefaultFramebuffer(bool depth) override;
			virtual void setColorMask(ColorMask mask) override;

			virtual ColorFormat getDeviceColorFormat() override;
		private:
			std::string vendor_name_;
			std::string adapter_name_;
			std::string api_version_;
		private:
#ifdef _WIN32
			HDC window_device_context_;
			HGLRC window_render_context_;
#endif
		};

		/*extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
			GRAPHICS_EXPORT void deleteGraphics(void *ptr);
		}*/
	}
}