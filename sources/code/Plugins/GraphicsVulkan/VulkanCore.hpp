#pragma once

#include <functional>
#include <Common/Logging.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/DLLDefs.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		struct QueueFamilyIndices {
			uint32_t graphicsFamily = 0;
			uint32_t presentFamily = 0;

			bool hasGraphicsFamily = false;
			bool hasPresentFamily = false;

			bool isComplete() {
				return hasGraphicsFamily && hasPresentFamily;
			}
		};

		class VulkanCore : public Core {
		public:
			VulkanCore(std::function<void(LogSeverity, const char*)> logFunction);
			virtual bool Initialize(Core::CreateInfo& ci) override;
			~VulkanCore();

			static VulkanCore * graphicsWrapper;
			static VulkanCore &Get();
			virtual void RegisterWindow(Window* window) override;
		public:
			QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
			uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			VkInstance GetInstance();
			VkDevice GetDevice();
			VkPhysicalDevice GetPhysicalDevice();
			VkCommandPool GetGraphicsCommandPool();
			std::function<void(LogSeverity, const char*)> logFunction;
		private:

			VkInstance instance = nullptr;
			VkDevice device = nullptr;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT debugMessenger;
			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			size_t currentFrame = 0;
		public:
			VkQueue graphicsQueue = nullptr;
			VkQueue presentQueue = nullptr;
			uint32_t graphicsFamily;
			uint32_t presentFamily;
			VkCommandPool commandPoolGraphics = nullptr;
			VkDescriptorPool descriptorPool = nullptr;
		private:
			void CreateInstance();
			void SetupDebugMessenger();
			void PickPhysicalDevice();
			void CreateLogicalDevice();
			void CreateCommandPool();
			void CreateDescriptorPool();
		private:
			uint16_t ScoreDevice(VkPhysicalDevice device);
			bool CheckValidationLayerSupport();
			std::vector<const char*> GetRequiredExtensions();
			void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		public:
			virtual const char* GetVendorName() override;
			virtual const char* GetAdapterName() override;
			virtual const char* GetAPIName() override;
			virtual const char* GetAPIVersion() override;

			virtual void AdjustPerspective(float *perspective) override;

			virtual void DeleteRenderTarget(RenderTarget * ptr) override;
			virtual void DeleteDepthTarget(DepthTarget * ptr) override;
			virtual void DeleteFramebuffer(Framebuffer *ptr) override;
			virtual void DeleteVertexBuffer(VertexBuffer *ptr) override;
			virtual void DeleteIndexBuffer(IndexBuffer *ptr) override;
			virtual void DeleteUniformBuffer(UniformBuffer * ptr) override;
			virtual void DeleteUniformBufferBinding(UniformBufferBinding * ptr) override;
			virtual void DeletePipeline(Pipeline *ptr) override;
			virtual void DeleteRenderPass(RenderPass *ptr) override;
			virtual void DeleteTexture(Texture *ptr) override;
			virtual void DeleteTextureBinding(TextureBinding *ptr) override;
			virtual void DeleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void DeleteCommandBuffer(CommandBuffer *ptr) override;
			virtual void DeleteVertexArrayObject(VertexArrayObject *ptr) override;

			virtual Framebuffer *CreateFramebuffer(Framebuffer::CreateInfo& ci) override;
			virtual RenderPass *CreateRenderPass(RenderPass::CreateInfo& ci) override;
			virtual Pipeline *CreatePipeline(Pipeline::CreateInfo& ci) override;
			virtual CommandBuffer *CreateCommandBuffer(CommandBuffer::CreateInfo& ci) override;
			virtual VertexArrayObject *CreateVertexArrayObject(VertexArrayObject::CreateInfo& ci) override;
			virtual VertexBuffer *CreateVertexBuffer(VertexBuffer::CreateInfo& ci) override;
			virtual IndexBuffer *CreateIndexBuffer(IndexBuffer::CreateInfo& ci) override;
			virtual UniformBuffer *CreateUniformBuffer(UniformBuffer::CreateInfo& ci) override;
			virtual UniformBufferBinding *CreateUniformBufferBinding(UniformBufferBinding::CreateInfo& ci) override;
			virtual Texture * CreateCubemap(Texture::CubemapCreateInfo& ci) override;
			virtual Texture *CreateTexture(Texture::CreateInfo& ci) override;
			virtual TextureBinding *CreateTextureBinding(TextureBinding::CreateInfo& ci) override;
			virtual TextureBindingLayout *CreateTextureBindingLayout(TextureBindingLayout::CreateInfo& ci) override;
			virtual RenderTarget* CreateRenderTarget(RenderTarget::CreateInfo& rt) override;
			virtual RenderTarget *CreateRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget *CreateDepthTarget(DepthTarget::CreateInfo& rt) override;
			
			virtual inline const bool ShouldUseImmediateMode() override;
			virtual inline const bool SupportsCommandBuffers() override;
			virtual inline const bool SupportsTesselation() override;
			virtual inline const bool SupportsGeometryShader() override;
			virtual inline const bool SupportsComputeShader() override;
			virtual inline const bool SupportsMultiDrawIndirect() override;

			virtual void WaitUntilIdle() override;

			// Unused
			virtual void Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
			virtual void BindTexture(TextureBinding*) override;
			virtual void BindPipeline(Pipeline*) override;
			virtual void BindVertexArrayObject(VertexArrayObject *) override;
			virtual	void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
			virtual void DrawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
			virtual void SetImmediateBlending(BlendMode) override;
			virtual void EnableDepthWrite(bool state) override;
			virtual void SetColorMask(ColorMask mask) override;
		private:
			std::string vendorName;
			std::string adapterName;
			std::string apiVersion;

			Window* primaryWindow = nullptr;

			// Inherited via Core
			virtual const char* GetDefaultShaderExtension() override;
			virtual void CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) override;
			virtual void BindDefaultFramebuffer() override;
			virtual void BindDefaultFramebufferWrite() override;
			virtual void BindDefaultFramebufferRead() override;
			virtual void ResizeViewport(uint32_t w, uint32_t h) override;
		};
	}
}
