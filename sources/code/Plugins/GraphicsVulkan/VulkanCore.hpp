#pragma once

#include <functional>

#include <vma/vk_mem_alloc.h>

#include <Common/Logging.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/DLLDefs.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		uint32_t graphicsFamily = 0;
		uint32_t presentFamily = 0;

		bool hasGraphicsFamily = false;
		bool hasPresentFamily = false;

		bool IsComplete() const {
			return hasGraphicsFamily && hasPresentFamily;
		}
	};

	class Core : public Grindstone::GraphicsAPI::Core {
	public:
		virtual bool Initialize(const Grindstone::GraphicsAPI::Core::CreateInfo& ci) override;
		~Core();

		static Vulkan::Core* graphicsWrapper;
		static Vulkan::Core& Get();
		virtual void RegisterWindow(Window* window) override;
	public:
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		virtual VkInstance GetInstance();
		virtual VkDevice GetDevice();
		virtual VkPhysicalDevice GetPhysicalDevice();
		virtual VkCommandBuffer BeginSingleTimeCommands();
		virtual uint32_t GetGraphicsFamily();
		virtual void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		VkCommandPool GetGraphicsCommandPool() const;
	private:

		VkInstance instance = nullptr;
		VkDevice device = nullptr;
		VkPhysicalDevice physicalDevice = nullptr;
		VkDebugUtilsMessengerEXT debugMessenger = nullptr;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
		PFN_vkSetDebugUtilsObjectNameEXT pfnDebugUtilsSetObjectName = nullptr;
	public:
		VkQueue graphicsQueue = nullptr;
		VkQueue presentQueue = nullptr;
		uint32_t graphicsFamily = 0;
		uint32_t presentFamily = 0;
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
		void CreateAllocator();
		uint16_t ScoreDevice(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	public:
		void NameObject(VkObjectType objectType, void* object, const char* objectName);

		virtual const char* GetVendorName() const override;
		virtual const char* GetAdapterName() const override;
		virtual const char* GetAPIName() const override;
		virtual const char* GetAPIVersion() const override;

		virtual void AdjustPerspective(float *perspective) override;

		virtual void DeleteFramebuffer(GraphicsAPI::Framebuffer *ptr) override;
		virtual void DeleteBuffer(GraphicsAPI::Buffer *ptr) override;
		virtual void DeleteGraphicsPipeline(GraphicsAPI::GraphicsPipeline* ptr) override;
		virtual void DeleteComputePipeline(GraphicsAPI::ComputePipeline* ptr) override;
		virtual void DeleteRenderPass(GraphicsAPI::RenderPass *ptr) override;
		virtual void DeleteSampler(GraphicsAPI::Sampler* ptr) override;
		virtual void DeleteImage(GraphicsAPI::Image* ptr) override;
		virtual void DeleteDescriptorSet(GraphicsAPI::DescriptorSet* ptr) override;
		virtual void DeleteDescriptorSetLayout(GraphicsAPI::DescriptorSetLayout* ptr) override;
		virtual void DeleteCommandBuffer(GraphicsAPI::CommandBuffer *ptr) override;
		virtual void DeleteVertexArrayObject(GraphicsAPI::VertexArrayObject *ptr) override;

		virtual GraphicsAPI::Framebuffer* CreateFramebuffer(const GraphicsAPI::Framebuffer::CreateInfo& ci) override;
		virtual GraphicsAPI::RenderPass* CreateRenderPass(const GraphicsAPI::RenderPass::CreateInfo& ci) override;
		virtual GraphicsAPI::GraphicsPipeline* CreateGraphicsPipeline(const GraphicsAPI::GraphicsPipeline::CreateInfo& ci) override;
		virtual GraphicsAPI::ComputePipeline* CreateComputePipeline(const GraphicsAPI::ComputePipeline::CreateInfo& ci) override;
		virtual GraphicsAPI::CommandBuffer* CreateCommandBuffer(const GraphicsAPI::CommandBuffer::CreateInfo& ci) override;
		virtual GraphicsAPI::VertexArrayObject* CreateVertexArrayObject(const GraphicsAPI::VertexArrayObject::CreateInfo& ci) override;
		virtual GraphicsAPI::Buffer* CreateBuffer(const GraphicsAPI::Buffer::CreateInfo& ci) override;
		virtual GraphicsAPI::Sampler* CreateSampler(const GraphicsAPI::Sampler::CreateInfo& ci) override;
		virtual GraphicsAPI::Image* CreateImage(const GraphicsAPI::Image::CreateInfo& ci) override;
		virtual GraphicsAPI::DescriptorSet* CreateDescriptorSet(const GraphicsAPI::DescriptorSet::CreateInfo& ci) override;
		virtual GraphicsAPI::DescriptorSetLayout* CreateDescriptorSetLayout(const GraphicsAPI::DescriptorSetLayout::CreateInfo& ci) override;
		
		virtual GraphicsAPI::GraphicsPipeline* GetOrCreateGraphicsPipelineFromCache(const GraphicsPipeline::PipelineData& pipelineData, const VertexInputLayout* vertexInputLayout) override;

		virtual inline bool ShouldUseImmediateMode() const override;
		virtual inline bool SupportsCommandBuffers() const override;
		virtual inline bool SupportsTesselation() const override;
		virtual inline bool SupportsGeometryShader() const override;
		virtual inline bool SupportsComputeShader() const override;
		virtual inline bool SupportsMultiDrawIndirect() const override;

		virtual void WaitUntilIdle() override;

		// Unused
		virtual void Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) override;
		virtual void BindGraphicsPipeline(GraphicsAPI::GraphicsPipeline*) override;
		virtual void BindVertexArrayObject(GraphicsAPI::VertexArrayObject *) override;
		virtual	void DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) override;
		virtual void DrawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) override;
		virtual void SetImmediateBlending(
			BlendOperation colorOp, BlendFactor colorSrc, BlendFactor colorDst,
			BlendOperation alphaOp, BlendFactor alphaSrc, BlendFactor alphaDst
		) override;
		virtual void EnableDepthWrite(bool state) override;
		virtual void SetColorMask(ColorMask mask) override;
	private:
		std::string vendorName;
		std::string adapterName;
		std::string apiVersion;
		VmaAllocator allocator;

		Window* primaryWindow = nullptr;

		// Inherited via Core
		virtual const char* GetDefaultShaderExtension() const override;
		virtual void CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) override;
		virtual void BindDefaultFramebuffer() override;
		virtual void BindDefaultFramebufferWrite() override;
		virtual void BindDefaultFramebufferRead() override;
		virtual void ResizeViewport(uint32_t w, uint32_t h) override;

		using PipelineHash = size_t;
		std::unordered_map<PipelineHash, Grindstone::GraphicsAPI::GraphicsPipeline*> graphicsPipelineCache;
	};
}
