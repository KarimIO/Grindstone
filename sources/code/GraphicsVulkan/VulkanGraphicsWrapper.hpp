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
			bool initialize(GraphicsWrapperCreateInfo ci);
			~VulkanGraphicsWrapper();

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

			virtual const char* getVendorName() override;
			virtual const char* getAdapterName() override;
			virtual const char* getAPIName() override;
			virtual const char* getAPIVersion() override;

			virtual void adjustPerspective(float *perspective) override;

			virtual void deleteRenderTarget(RenderTarget * ptr) override;
			virtual void deleteDepthTarget(DepthTarget * ptr) override;
			virtual void deleteFramebuffer(Framebuffer *ptr) override;
			virtual void deleteVertexBuffer(VertexBuffer *ptr) override;
			virtual void deleteIndexBuffer(IndexBuffer *ptr) override;
			virtual void deleteUniformBuffer(UniformBuffer * ptr) override;
			virtual void deleteUniformBufferBinding(UniformBufferBinding * ptr) override;
			virtual void deleteGraphicsPipeline(GraphicsPipeline *ptr) override;
			virtual void deleteRenderPass(RenderPass *ptr) override;
			virtual void deleteTexture(Texture *ptr) override;
			virtual void deleteTextureBinding(TextureBinding *ptr) override;
			virtual void deleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void deleteCommandBuffer(CommandBuffer *ptr) override;
			virtual void deleteVertexArrayObject(VertexArrayObject *ptr) override;

			virtual Framebuffer *createFramebuffer(FramebufferCreateInfo ci) override;
			virtual RenderPass *createRenderPass(RenderPassCreateInfo ci) override;
			virtual GraphicsPipeline *createGraphicsPipeline(GraphicsPipelineCreateInfo ci) override;
			virtual CommandBuffer *createCommandBuffer(CommandBufferCreateInfo ci) override;
			virtual VertexArrayObject *createVertexArrayObject(VertexArrayObjectCreateInfo ci) override;
			virtual VertexBuffer *createVertexBuffer(VertexBufferCreateInfo ci) override;
			virtual IndexBuffer *createIndexBuffer(IndexBufferCreateInfo ci) override;
			virtual UniformBuffer *createUniformBuffer(UniformBufferCreateInfo ci) override;
			virtual UniformBufferBinding *createUniformBufferBinding(UniformBufferBindingCreateInfo ci) override;
			virtual Texture *createCubemap(CubemapCreateInfo createInfo) override;
			virtual Texture *createTexture(TextureCreateInfo createInfo) override;
			virtual TextureBinding *createTextureBinding(TextureBindingCreateInfo ci) override;
			virtual TextureBindingLayout *createTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) override;
			virtual RenderTarget *createRenderTarget(RenderTargetCreateInfo *rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget *createDepthTarget(DepthTargetCreateInfo rt) override;
			
			virtual inline const bool shouldUseImmediateMode() override;
			virtual inline const bool supportsCommandBuffers() override;
			virtual inline const bool supportsTesselation() override;
			virtual inline const bool supportsGeometryShader() override;
			virtual inline const bool supportsComputeShader() override;
			virtual inline const bool supportsMultiDrawIndirect() override;

			virtual uint32_t getImageIndex() override;

			virtual void waitUntilIdle() override;
			virtual void drawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) override;

			virtual ColorFormat getDeviceColorFormat() override;

			// Unused
			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
			virtual void swapBuffers() override;
			virtual void setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override;
			virtual void bindTextureBinding(TextureBinding *) override;
			virtual void bindVertexArrayObject(VertexArrayObject *) override;
			virtual	void drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
			virtual void drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
			virtual void setImmediateBlending(BlendMode) override;
			virtual void enableDepth(bool state) override;
			virtual void setColorMask(ColorMask mask) override;
			virtual void copyToDepthBuffer(DepthTarget *p) override;
			virtual void bindDefaultFramebuffer(bool depth) override;
		private:
			std::string vendor_name_;
			std::string adapter_name_;
			std::string api_version_;
		};

		/*extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
			GRAPHICS_EXPORT void deleteGraphics(void *ptr);
		}*/
	}
}