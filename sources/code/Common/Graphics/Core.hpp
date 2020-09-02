#pragma once

#include <Common/Window/Window.hpp>

#include "IndexBuffer.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "VertexBuffer.hpp"
#include "UniformBuffer.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "VertexArrayObject.hpp"
#include "DepthTarget.hpp"

namespace Grindstone {
	class Window;
	namespace GraphicsAPI {
		enum class API {
			OpenGL = 0,
			Vulkan,
			DirectX11,
			DirectX12
		};

		class Core {
		public:
			struct CreateInfo {
				Window* window;
				bool debug;
			};

			virtual bool initialize(CreateInfo& createInfo) = 0;
			virtual void registerWindow(Window* window) = 0;
			API getAPI() {
				return api_type_;
			}
		public:
			virtual const char* getVendorName() = 0;
			virtual const char* getAdapterName() = 0;
			virtual const char* getAPIName() = 0;
			virtual const char* getAPIVersion() = 0;

			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) = 0;
			
			virtual void adjustPerspective(float *perspective) = 0;

			virtual void deleteRenderTarget(RenderTarget* ptr) = 0;
			virtual void deleteDepthTarget(DepthTarget* ptr) = 0;
			virtual void deleteFramebuffer(Framebuffer* ptr) = 0;
			virtual void deleteVertexBuffer(VertexBuffer* ptr) = 0;
			virtual void deleteIndexBuffer(IndexBuffer* ptr) = 0;
			virtual void deleteUniformBuffer(UniformBuffer* ptr) = 0;
			virtual void deleteUniformBufferBinding(UniformBufferBinding* ptr) = 0;
			virtual void deletePipeline(Pipeline* ptr) = 0;
			virtual void deleteRenderPass(RenderPass* ptr) = 0;
			virtual void deleteTexture(Texture* ptr) = 0;
			virtual void deleteTextureBinding(TextureBinding* ptr) = 0;
			virtual void deleteTextureBindingLayout(TextureBindingLayout* ptr) = 0;
			virtual void deleteCommandBuffer(CommandBuffer* ptr) = 0;
			virtual void deleteVertexArrayObject(VertexArrayObject* ptr) = 0;

			virtual Framebuffer* createFramebuffer(Framebuffer::CreateInfo& ci) = 0;
			virtual RenderPass* createRenderPass(RenderPass::CreateInfo& ci) = 0;
			virtual Pipeline* createPipeline(Pipeline::CreateInfo& ci) = 0;
			virtual CommandBuffer* createCommandBuffer(CommandBuffer::CreateInfo& ci) = 0;
			virtual VertexArrayObject* createVertexArrayObject(VertexArrayObject::CreateInfo& ci) = 0;
			virtual VertexBuffer* createVertexBuffer(VertexBuffer::CreateInfo& ci) = 0;
			virtual IndexBuffer* createIndexBuffer(IndexBuffer::CreateInfo& ci) = 0;
			virtual UniformBuffer* createUniformBuffer(UniformBuffer::CreateInfo& ci) = 0;
			virtual UniformBufferBinding* createUniformBufferBinding(UniformBufferBinding::CreateInfo& ci) = 0;
			virtual Texture* createCubemap(Texture::CubemapCreateInfo& createInfo) = 0;
			virtual Texture* createTexture(Texture::CreateInfo& createInfo) = 0;
			virtual TextureBinding* createTextureBinding(TextureBinding::CreateInfo& ci) = 0;
			virtual TextureBindingLayout* createTextureBindingLayout(TextureBindingLayout::CreateInfo& createInfo) = 0;
			virtual RenderTarget *createRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube = false) = 0;
			virtual DepthTarget* createDepthTarget(DepthTarget::CreateInfo& rt) = 0;

			virtual void copyToDepthBuffer(DepthTarget* p) = 0;

			virtual const bool shouldUseImmediateMode() = 0;
			virtual const bool supportsCommandBuffers() = 0;
			virtual const bool supportsTesselation() = 0;
			virtual const bool supportsGeometryShader() = 0;
			virtual const bool supportsComputeShader() = 0;
			virtual const bool supportsMultiDrawIndirect() = 0;

			virtual void bindDefaultFramebuffer(bool depth) = 0;

			virtual void waitUntilIdle() = 0;

			virtual void bindTexture(TextureBinding*) = 0;
			virtual void bindPipeline(Pipeline* pipeline) = 0;
			virtual void bindVertexArrayObject(VertexArrayObject*) = 0;
			virtual	void drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) = 0;
			virtual void drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) = 0;
			virtual void setImmediateBlending(BlendMode) = 0;
			virtual void enableDepth(bool state) = 0;
			virtual void setColorMask(ColorMask mask) = 0;

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
			bool debug_;
			API api_type_;
		};
	}
}