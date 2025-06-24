#pragma once

#include <Common/Window/Window.hpp>

#include "Buffer.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "ComputePipeline.hpp"
#include "CommandBuffer.hpp"
#include "VertexArrayObject.hpp"
#include "Image.hpp"
#include "Sampler.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"

namespace Grindstone {
	class Window;
}

namespace Grindstone::GraphicsAPI {
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

		virtual bool Initialize(const CreateInfo& createInfo) = 0;
		virtual void RegisterWindow(Window* window) = 0;
		API GetAPI() const {
			return apiType;
		}
	public:
		virtual const char* GetVendorName() const = 0;
		virtual const char* GetAdapterName() const = 0;
		virtual const char* GetAPIName() const = 0;
		virtual const char* GetAPIVersion() const = 0;
		virtual const char* GetDefaultShaderExtension() const = 0;

		virtual void Clear(ClearMode mask, float clearColor[4] = nullptr, float clearDepth = 0, uint32_t clearStencil = 0) = 0;

		virtual void AdjustPerspective(float *perspective) = 0;

		virtual void DeleteImage(Image* ptr) = 0;
		virtual void DeleteSampler(Sampler* ptr) = 0;
		virtual void DeleteFramebuffer(Framebuffer* ptr) = 0;
		virtual void DeleteBuffer(Buffer* ptr) = 0;
		virtual void DeleteGraphicsPipeline(GraphicsPipeline* ptr) = 0;
		virtual void DeleteComputePipeline(ComputePipeline* ptr) = 0;
		virtual void DeleteRenderPass(RenderPass* ptr) = 0;
		virtual void DeleteDescriptorSet(DescriptorSet* ptr) = 0;
		virtual void DeleteDescriptorSetLayout(DescriptorSetLayout* ptr) = 0;
		virtual void DeleteCommandBuffer(CommandBuffer* ptr) = 0;
		virtual void DeleteVertexArrayObject(VertexArrayObject* ptr) = 0;

		virtual Framebuffer* CreateFramebuffer(const Framebuffer::CreateInfo& ci) = 0;
		virtual RenderPass* CreateRenderPass(const RenderPass::CreateInfo& ci) = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipeline::CreateInfo& ci) = 0;
		virtual ComputePipeline* CreateComputePipeline(const ComputePipeline::CreateInfo& ci) = 0;
		virtual CommandBuffer* CreateCommandBuffer(const CommandBuffer::CreateInfo& ci) = 0;
		virtual VertexArrayObject* CreateVertexArrayObject(const VertexArrayObject::CreateInfo& ci) = 0;
		virtual Buffer* CreateBuffer(const Buffer::CreateInfo& ci) = 0;
		virtual Sampler* CreateSampler(const Sampler::CreateInfo& createInfo) = 0;
		virtual Image* CreateImage(const Image::CreateInfo& createInfo) = 0;
		virtual DescriptorSet* CreateDescriptorSet(const DescriptorSet::CreateInfo& ci) = 0;
		virtual DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayout::CreateInfo& ci) = 0;

		virtual GraphicsPipeline* GetOrCreateGraphicsPipelineFromCache(const GraphicsPipeline::PipelineData& pipelineData, const VertexInputLayout* vertexInputLayout) = 0;

		virtual void CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) = 0;

		virtual bool ShouldUseImmediateMode() const = 0;
		virtual bool SupportsCommandBuffers() const = 0;
		virtual bool SupportsTesselation() const = 0;
		virtual bool SupportsGeometryShader() const = 0;
		virtual bool SupportsComputeShader() const = 0;
		virtual bool SupportsMultiDrawIndirect() const = 0;

		virtual void BindDefaultFramebuffer() = 0;
		virtual void BindDefaultFramebufferWrite() = 0;
		virtual void BindDefaultFramebufferRead() = 0;

		virtual void WaitUntilIdle() = 0;

		virtual void BindGraphicsPipeline(GraphicsPipeline* pipeline) = 0;
		virtual void BindVertexArrayObject(VertexArrayObject*) = 0;
		virtual	void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) = 0;
		virtual void DrawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) = 0;
		virtual void SetImmediateBlending(
			BlendOperation colorOp, BlendFactor colorSrc, BlendFactor colorDst,
			BlendOperation alphaOp, BlendFactor alphaSrc, BlendFactor alphaDst
		) = 0;
		virtual void EnableDepthWrite(bool isDepthEnabled) = 0;
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
