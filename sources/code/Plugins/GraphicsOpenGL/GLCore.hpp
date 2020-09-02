#pragma once
#define NOMINMAX
#include <GL/gl3w.h>
#include <vector>

#include "GLRenderPass.hpp"
#include "GLPipeline.hpp"
#include "GLFramebuffer.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLUniformBuffer.hpp"
#include "GLTexture.hpp"
#include "GLDepthTarget.hpp"
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/DLLDefs.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		GLenum getGeomType(GeometryType geom_type);

		class GLCore : public Core {
		public:
			virtual bool initialize(Core::CreateInfo& createInfo) override;
			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
			virtual void adjustPerspective(float *perspective) override;
			virtual void registerWindow(Window* window) override;

			virtual const char* getVendorName() override;
			virtual const char* getAdapterName() override;
			virtual const char* getAPIName() override;
			virtual const char* getAPIVersion() override;

			virtual void deleteRenderTarget(RenderTarget * ptr) override;
			virtual void deleteDepthTarget(DepthTarget * ptr) override;
			virtual void deleteFramebuffer(Framebuffer *ptr) override;
			virtual void deleteVertexBuffer(VertexBuffer *ptr) override;
			virtual void deleteIndexBuffer(IndexBuffer *ptr) override;
			virtual void deleteUniformBuffer(UniformBuffer * ptr) override;
			virtual void deleteUniformBufferBinding(UniformBufferBinding * ptr) override;
			virtual void deletePipeline(Pipeline *ptr) override;
			virtual void deleteRenderPass(RenderPass *ptr) override;
			virtual void deleteTexture(Texture *ptr) override;
			virtual void deleteTextureBinding(TextureBinding *ptr) override;
			virtual void deleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void deleteCommandBuffer(CommandBuffer * ptr) override;
			virtual void deleteVertexArrayObject(VertexArrayObject *ptr) override;

			virtual Framebuffer *createFramebuffer(Framebuffer::CreateInfo& ci) override;
			virtual RenderPass *createRenderPass(RenderPass::CreateInfo& ci) override;
			virtual Pipeline *createPipeline(Pipeline::CreateInfo& ci) override;
			virtual CommandBuffer *createCommandBuffer(CommandBuffer::CreateInfo& gp) override;
			virtual VertexArrayObject *createVertexArrayObject(VertexArrayObject::CreateInfo& gp) override;
			virtual VertexBuffer *createVertexBuffer(VertexBuffer::CreateInfo& ci) override;
			virtual IndexBuffer *createIndexBuffer(IndexBuffer::CreateInfo& ci) override;
			virtual UniformBuffer *createUniformBuffer(UniformBuffer::CreateInfo& ci) override;
			virtual UniformBufferBinding *createUniformBufferBinding(UniformBufferBinding::CreateInfo& ci) override;
			virtual Texture *createCubemap(Texture::CubemapCreateInfo& createInfo) override;
			virtual Texture *createTexture(Texture::CreateInfo& createInfo) override;
			virtual TextureBinding *createTextureBinding(TextureBinding::CreateInfo& createInfo) override;
			virtual TextureBindingLayout *createTextureBindingLayout(TextureBindingLayout::CreateInfo& createInfo) override;
			virtual RenderTarget *createRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget *createDepthTarget(DepthTarget::CreateInfo& rt) override;

			virtual void copyToDepthBuffer(DepthTarget *p) override;

			virtual const bool shouldUseImmediateMode() override;
			virtual const bool supportsCommandBuffers() override;
			virtual const bool supportsTesselation() override;
			virtual const bool supportsGeometryShader() override;
			virtual const bool supportsComputeShader() override;
			virtual const bool supportsMultiDrawIndirect() override;

			virtual void waitUntilIdle() override;

			virtual void bindTexture(TextureBinding *) override;
			virtual void bindPipeline(Pipeline* pipeline) override;
			virtual void bindVertexArrayObject(VertexArrayObject *) override;
			virtual void drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
			virtual void drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
			virtual void setImmediateBlending(BlendMode) override;
			virtual void enableDepth(bool state) override;
			virtual void bindDefaultFramebuffer(bool depth) override;
			virtual void setColorMask(ColorMask mask) override;
		private:
			std::string vendor_name_;
			std::string adapter_name_;
			std::string api_version_;

			Window* primary_window_;
		};

		/*extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(Instance::CreateInfo& createInfo);
			GRAPHICS_EXPORT void deleteGraphics(void *ptr);
		}*/
	}
}