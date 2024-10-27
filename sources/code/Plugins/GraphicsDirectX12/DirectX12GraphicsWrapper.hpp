#pragma once

#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace Grindstone {
	namespace GraphicsAPI {
		const int num_buffers = 3;

		class DirectX12GraphicsWrapper : public GraphicsWrapper {
		public:
			bool initialize(GraphicsWrapperCreateInfo ci);
			~DirectX12GraphicsWrapper();

			static DirectX12GraphicsWrapper* graphics_wrapper_;
			static DirectX12GraphicsWrapper& get();
		public:
		private:

		public:
			ID3D12Fence* fence_[num_buffers];    // an object that is locked while our command list is being executed by the gpu. We need as many
			HANDLE fence_event_; // a handle to an event when our fence is unlocked by the gpu
			UINT64 fence_value_[num_buffers]; // this value is incremented each frame. each fence will have its own value
			int frame_index_;

			ID3D12CommandQueue* direct_command_queue_;
			ID3D12CommandQueue* compute_command_queue_;
			ID3D12CommandQueue* copy_command_queue_;

			ComPtr<IDXGIAdapter4> dxgi_adapter_;
			ComPtr<ID3D12Device2> device_;
			ComPtr<IDXGISwapChain4> swap_chain_;
			ComPtr<ID3D12CommandQueue> command_queue_;
			ID3D12CommandAllocator* graphics_command_allocator_;
			uint32_t current_backbuffer_index_;

			std::vector<RenderTarget *> swap_chain_targets_;
			ComPtr<ID3D12DescriptorHeap> rt_descriptor_heap_;
		private:
			void setupDebugMessenger();
			void createCommandQueues();
			void pickPhysicalDevice();
			void createLogicalDevice();
			void createSwapChain();
			void createCommandPool();
			void createDescriptorPool();
			void createSyncObjects();
		private:

		public:
			virtual const char* getVendorName() override;
			virtual const char* getAdapterName() override;
			virtual const char* getAPIName() override;
			virtual const char* getAPIVersion() override;

			virtual void adjustPerspective(float* perspective) override;

			void registerWindow(Window* window);

			virtual void deleteRenderTarget(RenderTarget* ptr) override;
			virtual void deleteDepthStencilTarget(DepthStencilTarget* ptr) override;
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
			virtual DepthStencilTarget* createDepthStencilTarget(DepthStencilTargetCreateInfo rt) override;

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
			virtual void copyToDepthBuffer(DepthStencilTarget* p) override;
			virtual void bindDefaultFramebuffer(bool depth) override;
		private:
			std::string vendor_name_;
			std::string adapter_name_;
			std::string api_version_;
		};
	}
}
