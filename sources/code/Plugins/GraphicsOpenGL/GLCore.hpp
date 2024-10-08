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
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/DLLDefs.hpp>

namespace Grindstone::GraphicsAPI {
	class GLCore : public Core {
	public:
		virtual bool Initialize(CreateInfo& createInfo) override;
		virtual void Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
		virtual void AdjustPerspective(float *perspective) override;
		virtual void RegisterWindow(Window* window) override;

		virtual const char* GetVendorName() override;
		virtual const char* GetAdapterName() override;
		virtual const char* GetAPIName() override;
		virtual const char* GetAPIVersion() override;
		virtual const char* GetDefaultShaderExtension() override;

		virtual void DeleteRenderTarget(RenderTarget * ptr) override;
		virtual void DeleteDepthTarget(DepthTarget * ptr) override;
		virtual void DeleteFramebuffer(Framebuffer *ptr) override;
		virtual void DeleteVertexBuffer(VertexBuffer *ptr) override;
		virtual void DeleteIndexBuffer(IndexBuffer *ptr) override;
		virtual void DeleteUniformBuffer(UniformBuffer * ptr) override;
		virtual void DeleteGraphicsPipeline(GraphicsPipeline* ptr) override;
		virtual void DeleteComputePipeline(ComputePipeline* ptr) override;
		virtual void DeleteRenderPass(RenderPass *ptr) override;
		virtual void DeleteTexture(Texture *ptr) override;
		virtual void DeleteDescriptorSet(DescriptorSet *ptr) override;
		virtual void DeleteDescriptorSetLayout(DescriptorSetLayout *ptr) override;
		virtual void DeleteCommandBuffer(CommandBuffer * ptr) override;
		virtual void DeleteVertexArrayObject(VertexArrayObject *ptr) override;

		virtual Framebuffer *CreateFramebuffer(Framebuffer::CreateInfo& ci) override;
		virtual RenderPass *CreateRenderPass(RenderPass::CreateInfo& ci) override;
		virtual GraphicsPipeline* CreateGraphicsPipeline(GraphicsPipeline::CreateInfo& ci) override;
		virtual ComputePipeline* CreateComputePipeline(ComputePipeline::CreateInfo& ci) override;
		virtual CommandBuffer *CreateCommandBuffer(CommandBuffer::CreateInfo& gp) override;
		virtual VertexArrayObject *CreateVertexArrayObject(VertexArrayObject::CreateInfo& gp) override;
		virtual VertexBuffer *CreateVertexBuffer(VertexBuffer::CreateInfo& ci) override;
		virtual IndexBuffer *CreateIndexBuffer(IndexBuffer::CreateInfo& ci) override;
		virtual UniformBuffer *CreateUniformBuffer(UniformBuffer::CreateInfo& ci) override;
		virtual Texture *CreateCubemap(Texture::CubemapCreateInfo& createInfo) override;
		virtual Texture *CreateTexture(Texture::CreateInfo& createInfo) override;
		virtual DescriptorSet *CreateDescriptorSet(DescriptorSet::CreateInfo& createInfo) override;
		virtual DescriptorSetLayout *CreateDescriptorSetLayout(DescriptorSetLayout::CreateInfo& createInfo) override;
		virtual RenderTarget* CreateRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube = false) override;
		virtual RenderTarget* CreateRenderTarget(RenderTarget::CreateInfo& rt) override;
		virtual DepthTarget *CreateDepthTarget(DepthTarget::CreateInfo& rt) override;

		virtual void CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) override;

		virtual const bool ShouldUseImmediateMode() override;
		virtual const bool SupportsCommandBuffers() override;
		virtual const bool SupportsTesselation() override;
		virtual const bool SupportsGeometryShader() override;
		virtual const bool SupportsComputeShader() override;
		virtual const bool SupportsMultiDrawIndirect() override;

		virtual void WaitUntilIdle() override;

		virtual void BindGraphicsPipeline(GraphicsPipeline* pipeline) override;
		virtual void BindVertexArrayObject(VertexArrayObject *) override;
		virtual void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
		virtual void DrawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
		virtual void SetImmediateBlending(
			BlendOperation colorOp, BlendFactor colorSrc, BlendFactor colorDst,
			BlendOperation alphaOp, BlendFactor alphaSrc, BlendFactor alphaDst
		) override;
		virtual void EnableDepthWrite(bool state) override;
		virtual void BindDefaultFramebuffer() override;
		virtual void BindDefaultFramebufferWrite() override;
		virtual void BindDefaultFramebufferRead() override;
		virtual void SetColorMask(ColorMask mask) override;
		virtual void ResizeViewport(uint32_t w, uint32_t h) override;
	private:
		std::string vendorName;
		std::string adapterName;
		std::string apiVersion;

		Window* primaryWindow;
	};
}
