#pragma once

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
			virtual bool Initialize(Core::CreateInfo& ci) override;
			~VulkanCore();

			static VulkanCore *graphics_wrapper_;
			static VulkanCore &get();
			virtual void RegisterWindow(Window* window) override;
		public:
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
			uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			VkInstance getInstance();
			VkDevice getDevice();
			VkPhysicalDevice getPhysicalDevice();
			VkCommandPool getGraphicsCommandPool();
		private:
			VkInstance instance_;
			VkDevice device_;
			VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT debug_messenger_;
			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			size_t currentFrame = 0;
		public:
			VkQueue graphics_queue_;
			VkQueue present_queue_;
			uint32_t graphics_family_;
			uint32_t present_family_;
			VkCommandPool command_pool_graphics_;
			VkDescriptorPool descriptor_pool_;
		private:
			void createInstance();
			void setupDebugMessenger();
			void pickPhysicalDevice();
			void createLogicalDevice();
			void createCommandPool();
			void createDescriptorPool();
		private:
			uint16_t scoreDevice(VkPhysicalDevice device);
			bool checkValidationLayerSupport();
			std::vector<const char*> getRequiredExtensions();
			void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
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
			virtual void EnableDepth(bool state) override;
			virtual void SetColorMask(ColorMask mask) override;
			virtual void CopyToDepthBuffer(DepthTarget *p) override;
			virtual void BindDefaultFramebuffer(bool depth) override;
		private:
			std::string vendor_name_;
			std::string adapter_name_;
			std::string api_version_;

			Window* primary_window_;
		};

		/*extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
			GRAPHICS_EXPORT void DeleteGraphics(void *ptr);
		}*/
	}
}