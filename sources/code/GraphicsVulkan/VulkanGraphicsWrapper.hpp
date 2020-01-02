#pragma once

#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		struct QueueFamilyIndices {
			uint32_t graphicsFamily;
			uint32_t presentFamily;

			bool hasGraphicsFamily;
			bool hasPresentFamily;

			bool isComplete() {
				return hasGraphicsFamily && hasPresentFamily;
			}
		};


		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		class VulkanGraphicsWrapper : public GraphicsWrapper {
		public:
			VulkanGraphicsWrapper(InstanceCreateInfo ci);
			virtual ~VulkanGraphicsWrapper() override;

			static VulkanGraphicsWrapper *graphics_wrapper_;
			static VulkanGraphicsWrapper &get();
		public:
			uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			VkDevice getDevice();
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
			VkSurfaceKHR surface_;
			VkQueue graphics_queue_;
			VkQueue present_queue_;
			uint32_t graphics_family_;
			uint32_t present_family_;
			VkCommandPool command_pool_graphics_;
			VkDescriptorPool descriptor_pool_;
			ColorFormat swapchain_format_;

			VkSwapchainKHR swap_chain_;
			std::vector<RenderTarget *> swap_chain_targets_;
		private:
			void createWindow();
			void createInstance();
			void setupDebugMessenger();
			void createSurface();
			void pickPhysicalDevice();
			void createLogicalDevice();
			void createSwapChain();
			void createCommandPool();
			void createDescriptorPool();
			void createSyncObjects();
		private:
			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
			uint16_t scoreDevice(VkPhysicalDevice device);
			bool checkValidationLayerSupport();
			std::vector<const char*> getRequiredExtensions();
			void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		public:
			virtual void getSwapChainRenderTargets(RenderTarget **&rts, uint32_t &rt_count) override;

			virtual void CreateDefaultStructures() override;

			virtual void DeleteRenderTarget(RenderTarget * ptr) override;
			virtual void DeleteDepthTarget(DepthTarget * ptr) override;
			virtual void DeleteFramebuffer(Framebuffer *ptr) override;
			virtual void DeleteVertexBuffer(VertexBuffer *ptr) override;
			virtual void DeleteIndexBuffer(IndexBuffer *ptr) override;
			virtual void DeleteUniformBuffer(UniformBuffer * ptr) override;
			virtual void DeleteUniformBufferBinding(UniformBufferBinding * ptr) override;
			virtual void DeleteGraphicsPipeline(GraphicsPipeline *ptr) override;
			virtual void DeleteRenderPass(RenderPass *ptr) override;
			virtual void DeleteTexture(Texture *ptr) override;
			virtual void DeleteTextureBinding(TextureBinding *ptr) override;
			virtual void DeleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void DeleteCommandBuffer(CommandBuffer *ptr) override;
			virtual void DeleteVertexArrayObject(VertexArrayObject *ptr) override;

			virtual Framebuffer *CreateFramebuffer(FramebufferCreateInfo ci) override;
			virtual RenderPass *CreateRenderPass(RenderPassCreateInfo ci) override;
			virtual GraphicsPipeline *CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci) override;
			virtual void CreateDefaultFramebuffers(DefaultFramebufferCreateInfo ci, Framebuffer **&framebuffers, uint32_t &framebufferCount) override;
			virtual CommandBuffer *CreateCommandBuffer(CommandBufferCreateInfo ci) override;
			virtual VertexArrayObject *CreateVertexArrayObject(VertexArrayObjectCreateInfo ci) override;
			virtual VertexBuffer *CreateVertexBuffer(VertexBufferCreateInfo ci) override;
			virtual IndexBuffer *CreateIndexBuffer(IndexBufferCreateInfo ci) override;
			virtual UniformBuffer *CreateUniformBuffer(UniformBufferCreateInfo ci) override;
			virtual UniformBufferBinding *CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci) override;
			virtual Texture *CreateCubemap(CubemapCreateInfo createInfo) override;
			virtual Texture *CreateTexture(TextureCreateInfo createInfo) override;
			virtual TextureBinding *CreateTextureBinding(TextureBindingCreateInfo ci) override;
			virtual TextureBindingLayout *CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) override;
			virtual RenderTarget *CreateRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget *CreateDepthTarget(DepthTargetCreateInfo rt) override;
			
			virtual inline const bool shouldUseImmediateMode() override;
			virtual inline const bool supportsCommandBuffers() override;
			virtual inline const bool supportsTesselation() override;
			virtual inline const bool supportsGeometryShader() override;
			virtual inline const bool supportsComputeShader() override;
			virtual inline const bool supportsMultiDrawIndirect() override;

			virtual uint32_t GetImageIndex() override;

			virtual void WaitUntilIdle() override;
			virtual void DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) override;

			virtual ColorFormat GetDeviceColorFormat() override;

			// Unused
			virtual void Clear(ClearMode mask) override;
			virtual void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override;
			virtual void BindTextureBinding(TextureBinding *) override;
			virtual void BindVertexArrayObject(VertexArrayObject *) override;
			virtual	void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
			virtual void DrawImmediateVertices(uint32_t base, uint32_t count) override;
			virtual void SetImmediateBlending(BlendMode) override;
			virtual void EnableDepth(bool state) override;
			virtual void SetColorMask(ColorMask mask) override;
			virtual void SwapBuffer() override;
			virtual void CopyToDepthBuffer(DepthTarget *p) override;
			virtual void BindDefaultFramebuffer(bool depth) override;
		};

		extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
			GRAPHICS_EXPORT void deleteGraphics(void *ptr);
		}
	}
}