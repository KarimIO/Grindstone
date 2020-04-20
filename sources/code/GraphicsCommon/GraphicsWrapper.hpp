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

class BaseWindow;

namespace Grindstone {
	namespace GraphicsAPI {
		enum class GraphicsAPIType {
			OpenGL = 0,
			Vulkan,
			DirectX11,
			DirectX12
		};

		struct GraphicsWrapperCreateInfo {
			BaseWindow* window;
			bool debug;
		};

		class GraphicsWrapper {
		public:
			virtual bool initialize(GraphicsWrapperCreateInfo createInfo) = 0;
			virtual void setWindow(BaseWindow* window);
			GraphicsAPIType getAPI() {
				return api_type_;
			}
		public:

			virtual void getSwapChainRenderTargets(RenderTarget**& rts, uint32_t& rt_count) = 0;
			virtual void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) = 0;

			virtual const char* getVendorName() = 0;
			virtual const char* getAdapterName() = 0;
			virtual const char* getAPIName() = 0;
			virtual const char* getAPIVersion() = 0;

			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) = 0;
			virtual void swapBuffers() = 0;

			virtual void adjustPerspective(float *perspective) = 0;

			virtual void deleteRenderTarget(RenderTarget* ptr) = 0;
			virtual void deleteDepthTarget(DepthTarget* ptr) = 0;
			virtual void deleteFramebuffer(Framebuffer* ptr) = 0;
			virtual void deleteVertexBuffer(VertexBuffer* ptr) = 0;
			virtual void deleteIndexBuffer(IndexBuffer* ptr) = 0;
			virtual void deleteUniformBuffer(UniformBuffer* ptr) = 0;
			virtual void deleteUniformBufferBinding(UniformBufferBinding* ptr) = 0;
			virtual void deleteGraphicsPipeline(GraphicsPipeline* ptr) = 0;
			virtual void deleteRenderPass(RenderPass* ptr) = 0;
			virtual void deleteTexture(Texture* ptr) = 0;
			virtual void deleteTextureBinding(TextureBinding* ptr) = 0;
			virtual void deleteTextureBindingLayout(TextureBindingLayout* ptr) = 0;
			virtual void deleteCommandBuffer(CommandBuffer* ptr) = 0;
			virtual void deleteVertexArrayObject(VertexArrayObject* ptr) = 0;

			virtual Framebuffer* createFramebuffer(FramebufferCreateInfo ci) = 0;
			virtual RenderPass* createRenderPass(RenderPassCreateInfo ci) = 0;
			virtual GraphicsPipeline* createGraphicsPipeline(GraphicsPipelineCreateInfo ci) = 0;
			virtual CommandBuffer* createCommandBuffer(CommandBufferCreateInfo ci) = 0;
			virtual VertexArrayObject* createVertexArrayObject(VertexArrayObjectCreateInfo ci) = 0;
			virtual VertexBuffer* createVertexBuffer(VertexBufferCreateInfo ci) = 0;
			virtual IndexBuffer* createIndexBuffer(IndexBufferCreateInfo ci) = 0;
			virtual UniformBuffer* createUniformBuffer(UniformBufferCreateInfo ci) = 0;
			virtual UniformBufferBinding* createUniformBufferBinding(UniformBufferBindingCreateInfo ci) = 0;
			virtual Texture* createCubemap(CubemapCreateInfo createInfo) = 0;
			virtual Texture* createTexture(TextureCreateInfo createInfo) = 0;
			virtual TextureBinding* createTextureBinding(TextureBindingCreateInfo ci) = 0;
			virtual TextureBindingLayout* createTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) = 0;
			virtual RenderTarget *createRenderTarget(RenderTargetCreateInfo* rt, uint32_t rc, bool cube = false) = 0;
			virtual DepthTarget* createDepthTarget(DepthTargetCreateInfo rt) = 0;

			virtual void copyToDepthBuffer(DepthTarget* p) = 0;

			virtual const bool shouldUseImmediateMode() = 0;
			virtual const bool supportsCommandBuffers() = 0;
			virtual const bool supportsTesselation() = 0;
			virtual const bool supportsGeometryShader() = 0;
			virtual const bool supportsComputeShader() = 0;
			virtual const bool supportsMultiDrawIndirect() = 0;

			virtual void bindDefaultFramebuffer(bool depth) = 0;

			virtual uint32_t getImageIndex() = 0;

			virtual void waitUntilIdle() = 0;
			virtual void drawCommandBuffers(uint32_t imageIndex, CommandBuffer** commandBuffers, uint32_t commandBufferCount) = 0;

			virtual void bindTextureBinding(TextureBinding*) = 0;
			virtual void bindVertexArrayObject(VertexArrayObject*) = 0;
			virtual	void drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) = 0;
			virtual void drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) = 0;
			virtual void setImmediateBlending(BlendMode) = 0;
			virtual void enableDepth(bool state) = 0;
			virtual void setColorMask(ColorMask mask) = 0;

			virtual ColorFormat getDeviceColorFormat() = 0;

			const char* getVendorNameFromID(uint32_t vendorID) {
				switch (vendorID) {
				case 0x1002:
					return "Advanced Micro Devices (AMD)";
					break;
				case 0x1010:
					return "Imagination Technologies";
					break;
				case 0x10DE:
					return "NVIDIA Corporation";
					break;
				case 0x13B5:
					return "Arm Limited";
					break;
				case 0x5143:
					return "Qualcomm Technologies, Inc.";
					break;
				case 0x163C:
				case 0x8086:
				case 0x8087:
					return "Intel Corporation";
					break;
				default:
					return 0;
				}
			};
		protected:
			BaseWindow* window_;
			bool debug_;
			GraphicsAPIType api_type_;
		};

		inline void GraphicsWrapper::setWindow(BaseWindow* window) {
			window_ = window;
		}
	}
}