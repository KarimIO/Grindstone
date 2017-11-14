#pragma once
#define NOMINMAX
#include <vulkan/vulkan.h>
#include <vector>

#include "VkRenderPass.h"
#include "VkGraphicsPipeline.h"
#include "VkFramebuffer.h"
#include "VkCommandBuffer.h"
#include "VkVertexBuffer.h"
#include "VkIndexBuffer.h"
#include "VkUniformBuffer.h"
#include "VkTexture.h"
#include "../GraphicsCommon/GraphicsWrapper.h"
#include "../GraphicsCommon/DLLDefs.h"
#include "../GraphicsCommon/VertexArrayObject.h"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilies {
	int graphics = -1;
	int present = -1;
	bool isComplete();
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class GRAPHICS_EXPORT_CLASS VkGraphicsWrapper : public GraphicsWrapper {
private:
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkInstance instance;

	VkPhysicalDeviceFeatures deviceFeatures;

	VkSurfaceKHR vkSurface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	QueueFamilies indices;
	VkFormat swapChainImageFormat;
	VkExtent2D resolution;

	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkDescriptorPool descriptorPool;
	VkCommandPool commandPool;

	VkSwapchainKHR swapChain;

	bool vsync;
	bool debug;

	void InitializeVulkan();
	void SetupDebugCallback();
	bool CheckValidationLayerSupport();

	void PickDevice();
	void CreateLogicalDevice();
	int	 DeviceScore(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilies findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void CreateSwapChain();

	bool vkInitializeHandleResult(VkResult result);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

	VkDebugReportCallbackEXT callback;
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

	void CreateDefaultCommandPool();
	void CreateDefaultDescriptorPool();
public:
	void Clear();

	VkGraphicsWrapper(InstanceCreateInfo createInfo);
	void CreateDefaultStructures();
	void Cleanup();

	void DeleteFramebuffer(Framebuffer *ptr);
	void DeleteVertexBuffer(VertexBuffer *ptr);
	void DeleteIndexBuffer(IndexBuffer *ptr);
	void DeleteUniformBuffer(UniformBuffer * ptr);
	void DeleteUniformBufferBinding(UniformBufferBinding * ptr);
	void DeleteGraphicsPipeline(GraphicsPipeline *ptr);
	void DeleteRenderPass(RenderPass *ptr);
	void DeleteTexture(Texture *ptr);
	void DeleteCommandBuffer(CommandBuffer * ptr);
	void DeleteVertexArrayObject(VertexArrayObject *ptr);

	Texture *CreateCubemap(CubemapCreateInfo createInfo);
	TextureBindingLayout *CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo);
	Framebuffer *CreateFramebuffer(FramebufferCreateInfo ci);
	RenderPass *CreateRenderPass(RenderPassCreateInfo ci);
	GraphicsPipeline *CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci);
	void CreateDefaultFramebuffers(DefaultFramebufferCreateInfo ci, Framebuffer **&framebuffers, uint32_t &framebufferCount);
	CommandBuffer *CreateCommandBuffer(CommandBufferCreateInfo ci);
	VertexArrayObject *CreateVertexArrayObject(VertexArrayObjectCreateInfo ci);
	VertexBuffer *CreateVertexBuffer(VertexBufferCreateInfo ci);
	IndexBuffer *CreateIndexBuffer(IndexBufferCreateInfo ci);
	UniformBuffer *CreateUniformBuffer(UniformBufferCreateInfo ci);
	UniformBufferBinding *CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci);
	Texture *CreateTexture(TextureCreateInfo createInfo);
	TextureBinding *CreateTextureBinding(TextureBindingCreateInfo ci);

	bool SupportsCommandBuffers();
	bool SupportsTesselation();
	bool SupportsGeometryShader();
	bool SupportsComputeShader();
	bool SupportsMultiDrawIndirect();

	uint32_t GetImageIndex();

	void WaitUntilIdle();
	void DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount);

	void BindTextureBinding(TextureBinding *);
	void BindVertexArrayObject(VertexArrayObject *);
	void DrawImmediateIndexed(bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount);
	void DrawImmediateVertices(uint32_t base, uint32_t count);
	void SetImmediateBlending(bool);

	ColorFormat GetDeviceColorFormat();

	void SwapBuffer();

	// VK-Access
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};

extern "C" GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo);
extern "C" GRAPHICS_EXPORT void deleteGraphics(void *ptr);