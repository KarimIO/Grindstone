#pragma once

#define NOMINMAX
#include <GL/gl3w.h>
#include <vector>
#include <unordered_map>

#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/DLLDefs.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class Core : public Grindstone::GraphicsAPI::Core {
	public:
		virtual bool Initialize(const CreateInfo& createInfo) override;
		virtual void Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
		virtual void AdjustPerspective(float *perspective) override;
		virtual void RegisterWindow(Window* window) override;

		virtual const char* GetVendorName() const override;
		virtual const char* GetAdapterName() const override;
		virtual const char* GetAPIName() const override;
		virtual const char* GetAPIVersion() const override;
		virtual const char* GetDefaultShaderExtension() const override;

		virtual void DeleteRenderTarget(Grindstone::GraphicsAPI::RenderTarget * ptr) override;
		virtual void DeleteDepthStencilTarget(Grindstone::GraphicsAPI::DepthStencilTarget * ptr) override;
		virtual void DeleteFramebuffer(Grindstone::GraphicsAPI::Framebuffer *ptr) override;
		virtual void DeleteBuffer(Grindstone::GraphicsAPI::Buffer *ptr) override;
		virtual void DeleteGraphicsPipeline(Grindstone::GraphicsAPI::GraphicsPipeline* ptr) override;
		virtual void DeleteComputePipeline(Grindstone::GraphicsAPI::ComputePipeline* ptr) override;
		virtual void DeleteRenderPass(Grindstone::GraphicsAPI::RenderPass *ptr) override;
		virtual void DeleteTexture(Grindstone::GraphicsAPI::Texture *ptr) override;
		virtual void DeleteDescriptorSet(Grindstone::GraphicsAPI::DescriptorSet *ptr) override;
		virtual void DeleteDescriptorSetLayout(Grindstone::GraphicsAPI::DescriptorSetLayout *ptr) override;
		virtual void DeleteCommandBuffer(Grindstone::GraphicsAPI::CommandBuffer * ptr) override;
		virtual void DeleteVertexArrayObject(Grindstone::GraphicsAPI::VertexArrayObject *ptr) override;

		virtual Grindstone::GraphicsAPI::Framebuffer* CreateFramebuffer(const Framebuffer::CreateInfo& ci) override;
		virtual Grindstone::GraphicsAPI::RenderPass* CreateRenderPass(const RenderPass::CreateInfo& ci) override;
		virtual Grindstone::GraphicsAPI::GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipeline::CreateInfo& ci) override;
		virtual Grindstone::GraphicsAPI::ComputePipeline* CreateComputePipeline(const ComputePipeline::CreateInfo& ci) override;
		virtual Grindstone::GraphicsAPI::CommandBuffer* CreateCommandBuffer(const CommandBuffer::CreateInfo& ci) override;
		virtual Grindstone::GraphicsAPI::VertexArrayObject* CreateVertexArrayObject(const VertexArrayObject::CreateInfo& gp) override;
		virtual Grindstone::GraphicsAPI::Buffer* CreateBuffer(const Buffer::CreateInfo& ci) override;
		virtual Grindstone::GraphicsAPI::Texture* CreateCubemap(const Texture::CubemapCreateInfo& createInfo) override;
		virtual Grindstone::GraphicsAPI::Texture* CreateTexture(const Texture::CreateInfo& createInfo) override;
		virtual Grindstone::GraphicsAPI::DescriptorSet* CreateDescriptorSet(const DescriptorSet::CreateInfo& createInfo) override;
		virtual Grindstone::GraphicsAPI::DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayout::CreateInfo& createInfo) override;
		virtual Grindstone::GraphicsAPI::RenderTarget* CreateRenderTarget(const RenderTarget::CreateInfo& rt) override;
		virtual Grindstone::GraphicsAPI::DepthStencilTarget* CreateDepthStencilTarget(const DepthStencilTarget::CreateInfo& rt) override;

		virtual Grindstone::GraphicsAPI::GraphicsPipeline* GetOrCreateGraphicsPipelineFromCache(const GraphicsPipeline::PipelineData& pipelineData, const VertexInputLayout* vertexInputLayout) override;

		virtual void CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) override;

		virtual bool ShouldUseImmediateMode() const override;
		virtual bool SupportsCommandBuffers() const override;
		virtual bool SupportsTesselation() const override;
		virtual bool SupportsGeometryShader() const override;
		virtual bool SupportsComputeShader() const override;
		virtual bool SupportsMultiDrawIndirect() const override;

		virtual void WaitUntilIdle() override;

		virtual void BindGraphicsPipeline(Grindstone::GraphicsAPI::GraphicsPipeline* pipeline) override;
		virtual void BindVertexArrayObject(Grindstone::GraphicsAPI::VertexArrayObject *) override;
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

		using PipelineHash = size_t;
		std::unordered_map<PipelineHash, Grindstone::GraphicsAPI::GraphicsPipeline*> graphicsPipelineCache;
	};
}
