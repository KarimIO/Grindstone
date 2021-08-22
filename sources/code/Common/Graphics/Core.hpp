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

			virtual bool Initialize(CreateInfo& createInfo) = 0;
			virtual void RegisterWindow(Window* window) = 0;
			API GetAPI() {
				return apiType;
			}
		public:
			virtual const char* GetVendorName() = 0;
			virtual const char* GetAdapterName() = 0;
			virtual const char* GetAPIName() = 0;
			virtual const char* GetAPIVersion() = 0;
			virtual const char* GetDefaultShaderExtension() = 0;

			virtual void Clear(ClearMode mask, float clearColor[4] = nullptr, float clearDepth = 0, uint32_t clearStencil = 0) = 0;
			
			virtual void AdjustPerspective(float *perspective) = 0;

			virtual void DeleteRenderTarget(RenderTarget* ptr) = 0;
			virtual void DeleteDepthTarget(DepthTarget* ptr) = 0;
			virtual void DeleteFramebuffer(Framebuffer* ptr) = 0;
			virtual void DeleteVertexBuffer(VertexBuffer* ptr) = 0;
			virtual void DeleteIndexBuffer(IndexBuffer* ptr) = 0;
			virtual void DeleteUniformBuffer(UniformBuffer* ptr) = 0;
			virtual void DeleteUniformBufferBinding(UniformBufferBinding* ptr) = 0;
			virtual void DeletePipeline(Pipeline* ptr) = 0;
			virtual void DeleteRenderPass(RenderPass* ptr) = 0;
			virtual void DeleteTexture(Texture* ptr) = 0;
			virtual void DeleteTextureBinding(TextureBinding* ptr) = 0;
			virtual void DeleteTextureBindingLayout(TextureBindingLayout* ptr) = 0;
			virtual void DeleteCommandBuffer(CommandBuffer* ptr) = 0;
			virtual void DeleteVertexArrayObject(VertexArrayObject* ptr) = 0;

			virtual Framebuffer* CreateFramebuffer(Framebuffer::CreateInfo& ci) = 0;
			virtual RenderPass* CreateRenderPass(RenderPass::CreateInfo& ci) = 0;
			virtual Pipeline* CreatePipeline(Pipeline::CreateInfo& ci) = 0;
			virtual CommandBuffer* CreateCommandBuffer(CommandBuffer::CreateInfo& ci) = 0;
			virtual VertexArrayObject* CreateVertexArrayObject(VertexArrayObject::CreateInfo& ci) = 0;
			virtual VertexBuffer* CreateVertexBuffer(VertexBuffer::CreateInfo& ci) = 0;
			virtual IndexBuffer* CreateIndexBuffer(IndexBuffer::CreateInfo& ci) = 0;
			virtual UniformBuffer* CreateUniformBuffer(UniformBuffer::CreateInfo& ci) = 0;
			virtual UniformBufferBinding* CreateUniformBufferBinding(UniformBufferBinding::CreateInfo& ci) = 0;
			virtual Texture* CreateCubemap(Texture::CubemapCreateInfo& createInfo) = 0;
			virtual Texture* CreateTexture(Texture::CreateInfo& createInfo) = 0;
			virtual TextureBinding* CreateTextureBinding(TextureBinding::CreateInfo& ci) = 0;
			virtual TextureBindingLayout* CreateTextureBindingLayout(TextureBindingLayout::CreateInfo& createInfo) = 0;
			virtual RenderTarget* CreateRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube = false) = 0;
			virtual DepthTarget* CreateDepthTarget(DepthTarget::CreateInfo& rt) = 0;

			virtual void CopyToDepthBuffer(DepthTarget* p) = 0;

			virtual const bool ShouldUseImmediateMode() = 0;
			virtual const bool SupportsCommandBuffers() = 0;
			virtual const bool SupportsTesselation() = 0;
			virtual const bool SupportsGeometryShader() = 0;
			virtual const bool SupportsComputeShader() = 0;
			virtual const bool SupportsMultiDrawIndirect() = 0;

			virtual void BindDefaultFramebuffer(bool isUsingDepth) = 0;
			virtual void BindDefaultFramebufferWrite(bool isUsingDepth) = 0;
			virtual void BindDefaultFramebufferRead() = 0;

			virtual void WaitUntilIdle() = 0;

			virtual void BindTexture(TextureBinding*) = 0;
			virtual void BindPipeline(Pipeline* pipeline) = 0;
			virtual void BindVertexArrayObject(VertexArrayObject*) = 0;
			virtual	void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) = 0;
			virtual void DrawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) = 0;
			virtual void SetImmediateBlending(BlendMode) = 0;
			virtual void EnableDepth(bool state) = 0;
			virtual void SetColorMask(ColorMask mask) = 0;
			virtual void ResizeViewport(uint32_t w, uint32_t h) = 0;

			const char* GetVendorNameFromID(uint32_t vendorID) {
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
			bool debug;
			API apiType;
		};
	}
}