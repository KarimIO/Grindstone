#pragma once

#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include <d3d11.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11GraphicsWrapper : public GraphicsWrapper {
		public:
			bool initialize(GraphicsWrapperCreateInfo ci);
			~DirectX11GraphicsWrapper();

			static DirectX11GraphicsWrapper* graphics_wrapper_;
			static DirectX11GraphicsWrapper& get();
		public:
		private:

		public:
			IDXGISwapChain* swap_chain_;             // the pointer to the swap chain interface
			ID3D11Device* device_;                     // the pointer to our Direct3D device interface
			ID3D11DeviceContext* device_context_;
			ComPtr<IDXGIAdapter> adapter_;
			ID3D11RenderTargetView* backbuffer_;
			ID3D11DepthStencilView* depth_stencil_view_;
			ID3D11Texture2D* depth_stencil_buffer_;
		private:
			void pickPhysicalDevice();
			void createLogicalDeviceAndSwapChain();
		private:
			
		public:

			virtual const char* getVendorName() override;
			virtual const char* getAdapterName() override;
			virtual const char* getAPIName() override;
			virtual const char* getAPIVersion() override;

			virtual void adjustPerspective(float* perspective) override;

			virtual void deleteRenderTarget(RenderTarget* ptr) override;
			virtual void deleteDepthTarget(DepthTarget* ptr) override;
			virtual void deleteFramebuffer(Framebuffer* ptr) override;
			virtual void deleteVertexBuffer(VertexBuffer* ptr) override;
			virtual void deleteIndexBuffer(IndexBuffer* ptr) override;
			virtual void deleteUniformBuffer(UniformBuffer* ptr) override;
			virtual void deleteUniformBufferBinding(UniformBufferBinding* ptr) override;
			virtual void deleteGraphicsPipeline(GraphicsPipeline* ptr) override;
			virtual void deleteRenderPass(RenderPass* ptr) override;
			virtual void deleteTexture(Texture* ptr) override;
			virtual void deleteTextureBinding(TextureBinding* ptr) override;
			virtual void deleteTextureBindingLayout(TextureBindingLayout* ptr) override;
			virtual void deleteCommandBuffer(CommandBuffer* ptr) override;
			virtual void deleteVertexArrayObject(VertexArrayObject* ptr) override;

			virtual Framebuffer* createFramebuffer(FramebufferCreateInfo ci) override;
			virtual RenderPass* createRenderPass(RenderPassCreateInfo ci) override;
			virtual GraphicsPipeline* createGraphicsPipeline(GraphicsPipelineCreateInfo ci) override;
			virtual CommandBuffer* createCommandBuffer(CommandBufferCreateInfo ci) override;
			virtual VertexArrayObject* createVertexArrayObject(VertexArrayObjectCreateInfo ci) override;
			virtual VertexBuffer* createVertexBuffer(VertexBufferCreateInfo ci) override;
			virtual IndexBuffer* createIndexBuffer(IndexBufferCreateInfo ci) override;
			virtual UniformBuffer* createUniformBuffer(UniformBufferCreateInfo ci) override;
			virtual UniformBufferBinding* createUniformBufferBinding(UniformBufferBindingCreateInfo ci) override;
			virtual Texture* createCubemap(CubemapCreateInfo createInfo) override;
			virtual Texture* createTexture(TextureCreateInfo createInfo) override;
			virtual TextureBinding* createTextureBinding(TextureBindingCreateInfo ci) override;
			virtual TextureBindingLayout* createTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) override;
			virtual RenderTarget* createRenderTarget(RenderTargetCreateInfo* rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget* createDepthTarget(DepthTargetCreateInfo rt) override;

			virtual inline const bool shouldUseImmediateMode() override;
			virtual inline const bool supportsCommandBuffers() override;
			virtual inline const bool supportsTesselation() override;
			virtual inline const bool supportsGeometryShader() override;
			virtual inline const bool supportsComputeShader() override;
			virtual inline const bool supportsMultiDrawIndirect() override;
			
			virtual void waitUntilIdle() override;

			// Unused
			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
			virtual void bindTextureBinding(TextureBinding*) override;
			virtual void bindVertexArrayObject(VertexArrayObject*) override;
			virtual	void drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
			virtual void drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
			virtual void setImmediateBlending(BlendMode) override;
			virtual void enableDepth(bool state) override;
			virtual void setColorMask(ColorMask mask) override;
			virtual void copyToDepthBuffer(DepthTarget* p) override;
			virtual void bindDefaultFramebuffer(bool depth) override;
		private:
			std::string vendor_name_;
			std::string adapter_name_;
			std::string api_version_;
		};
	}
}