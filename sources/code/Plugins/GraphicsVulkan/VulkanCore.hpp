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
			virtual bool initialize(Core::CreateInfo& ci) override;
			~VulkanCore();

			static VulkanCore *graphics_wrapper_;
			static VulkanCore &get();
			virtual void registerWindow(Window* window) override;
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
			virtual void deletePipeline(Pipeline *ptr) override;
			virtual void deleteRenderPass(RenderPass *ptr) override;
			virtual void deleteTexture(Texture *ptr) override;
			virtual void deleteTextureBinding(TextureBinding *ptr) override;
			virtual void deleteTextureBindingLayout(TextureBindingLayout *ptr) override;
			virtual void deleteCommandBuffer(CommandBuffer *ptr) override;
			virtual void deleteVertexArrayObject(VertexArrayObject *ptr) override;

			virtual Framebuffer *createFramebuffer(Framebuffer::CreateInfo& ci) override;
			virtual RenderPass *createRenderPass(RenderPass::CreateInfo& ci) override;
			virtual Pipeline *createPipeline(Pipeline::CreateInfo& ci) override;
			virtual CommandBuffer *createCommandBuffer(CommandBuffer::CreateInfo& ci) override;
			virtual VertexArrayObject *createVertexArrayObject(VertexArrayObject::CreateInfo& ci) override;
			virtual VertexBuffer *createVertexBuffer(VertexBuffer::CreateInfo& ci) override;
			virtual IndexBuffer *createIndexBuffer(IndexBuffer::CreateInfo& ci) override;
			virtual UniformBuffer *createUniformBuffer(UniformBuffer::CreateInfo& ci) override;
			virtual UniformBufferBinding *createUniformBufferBinding(UniformBufferBinding::CreateInfo& ci) override;
			virtual Texture * createCubemap(Texture::CubemapCreateInfo& createInfo) override;
			virtual Texture *createTexture(Texture::CreateInfo& ci) override;
			virtual TextureBinding *createTextureBinding(TextureBinding::CreateInfo& ci) override;
			virtual TextureBindingLayout *createTextureBindingLayout(TextureBindingLayout::CreateInfo& ci) override;
			virtual RenderTarget *createRenderTarget(RenderTarget::CreateInfo* rt, uint32_t rc, bool cube = false) override;
			virtual DepthTarget *createDepthTarget(DepthTarget::CreateInfo& rt) override;
			
			virtual inline const bool shouldUseImmediateMode() override;
			virtual inline const bool supportsCommandBuffers() override;
			virtual inline const bool supportsTesselation() override;
			virtual inline const bool supportsGeometryShader() override;
			virtual inline const bool supportsComputeShader() override;
			virtual inline const bool supportsMultiDrawIndirect() override;

			virtual void waitUntilIdle() override;

			// Unused
			virtual void clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
			virtual void bindTexture(TextureBinding*) override;
			virtual void bindPipeline(Pipeline*) override;
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

			Window* primary_window_;
		};

		/*extern "C" {
			GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
			GRAPHICS_EXPORT void deleteGraphics(void *ptr);
		}*/
	}
}