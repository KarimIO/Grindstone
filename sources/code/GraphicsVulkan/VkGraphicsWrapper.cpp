#include "VkGraphicsWrapper.h"
#include <iostream>
#include <array>
#include <assert.h>
#include <limits>
#include <cstring>

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkGraphicsWrapper::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData) {
	std::cerr << "VULKAN VALIDATION: " << msg << std::endl;

	return VK_FALSE;
}

void VkGraphicsWrapper::SetupDebugCallback() {
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
		throw std::runtime_error("VULKAN: Failed to set up debug callback!");
	}
}

void VkGraphicsWrapper::Clear() {
	std::cout << "Clear is not used in VkGraphicsWrapper\n";
	assert(true);
}

VkGraphicsWrapper::VkGraphicsWrapper(InstanceCreateInfo createInfo) {
	debug = createInfo.debug;
	width = createInfo.width;
	height = createInfo.height;
	vsync = createInfo.vsync;

	InitializeVulkan();

	if (debug)
		SetupDebugCallback();

#ifdef VK_USE_PLATFORM_WIN32_KHR
	InitializeWin32Window();

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = window->GetHandle();
	surfaceCreateInfo.pNext = VK_NULL_HANDLE;
	surfaceCreateInfo.flags = 0;

	if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &vkSurface) != VK_SUCCESS)
		return;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
	surfaceCreateInfo.dpy = xDisplay;
	surfaceCreateInfo.window = xWindow;
	surfaceCreateInfo.pNext = VK_NULL_HANDLE;
	surfaceCreateInfo.flags = 0;

	if (vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &vkSurface) != VK_SUCCESS)
		return;
#endif

	PickDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	std::cout << "Vulkan is Initialized\n";
}

void VkGraphicsWrapper::CreateDefaultDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 2;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 3;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VkGraphicsWrapper::CreateDefaultStructures() {
	CreateDefaultCommandPool();
	CreateDefaultDescriptorPool();

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {

		throw std::runtime_error("failed to create semaphores!");
	}
}


void VkGraphicsWrapper::CreateDefaultFramebuffers(DefaultFramebufferCreateInfo createInfo, Framebuffer **&framebuffers, uint32_t &framebufferCount) {
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

	framebuffers = new Framebuffer *[imageCount];

	for (size_t i = 0; i < imageCount; i++) {
		framebuffers[i] = new vkFramebuffer(&device, &physicalDevice, images[i], swapChainImageFormat, createInfo);
	}

	framebufferCount = imageCount;
}

uint32_t VkGraphicsWrapper::GetImageIndex() {
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	return imageIndex;
}

void VkGraphicsWrapper::CreateDefaultCommandPool() {
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = indices.graphics;
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

CommandBuffer *VkGraphicsWrapper::CreateCommandBuffer(CommandBufferCreateInfo ci) {
	return dynamic_cast<CommandBuffer *>(new vkCommandBuffer(&device, &commandPool, ci));
}

VertexArrayObject * VkGraphicsWrapper::CreateVertexArrayObject(VertexArrayObjectCreateInfo ci) {
	return nullptr;
}

VertexBuffer *VkGraphicsWrapper::CreateVertexBuffer(VertexBufferCreateInfo ci) {
	return dynamic_cast<VertexBuffer *>(new vkVertexBuffer(this, &device, &physicalDevice, ci));
}

IndexBuffer *VkGraphicsWrapper::CreateIndexBuffer(IndexBufferCreateInfo ci) {
	return dynamic_cast<IndexBuffer *>(new vkIndexBuffer(this, &device, &physicalDevice, ci));
}

UniformBuffer * VkGraphicsWrapper::CreateUniformBuffer(UniformBufferCreateInfo ci) {
	return dynamic_cast<UniformBuffer *>(new vkUniformBuffer(&device, &physicalDevice, &descriptorPool, ci));
}

UniformBufferBinding *VkGraphicsWrapper::CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
	return static_cast<UniformBufferBinding *>(new vkUniformBufferBinding(&device, ci));
}

Texture * VkGraphicsWrapper::CreateTexture(TextureCreateInfo createInfo) {
	return dynamic_cast<Texture *>(new vkTexture(&device, &physicalDevice, &commandPool, &descriptorPool, &graphicsQueue, createInfo));
}

TextureBinding * VkGraphicsWrapper::CreateTextureBinding(TextureBindingCreateInfo createInfo) {
	return dynamic_cast<TextureBinding *>(new vkTextureBinding(&device, &descriptorPool, createInfo));
}

bool VkGraphicsWrapper::SupportsCommandBuffers() {
	return true;
}

bool VkGraphicsWrapper::SupportsTesselation() {
	return deviceFeatures.tessellationShader;
}

bool VkGraphicsWrapper::SupportsGeometryShader() {
	return deviceFeatures.geometryShader;
}

bool VkGraphicsWrapper::SupportsComputeShader() {
	return true;
}

bool VkGraphicsWrapper::SupportsMultiDrawIndirect() {
	return deviceFeatures.multiDrawIndirect;
}

void VkGraphicsWrapper::WaitUntilIdle() {
	vkDeviceWaitIdle(device);
}

void VkGraphicsWrapper::DrawCommandBuffers(uint32_t imageIndex, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;


	std::vector<VkCommandBuffer> cbuffers;
	cbuffers.reserve(commandBufferCount);
	for (size_t i = 0; i < commandBufferCount; i++) {
		cbuffers.push_back(*static_cast<vkCommandBuffer *>(commandBuffers[i])->GetCommandBuffer(imageIndex));
	}
	submitInfo.commandBufferCount = cbuffers.size();
	submitInfo.pCommandBuffers = cbuffers.data();

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(presentQueue, &presentInfo);

	vkQueueWaitIdle(presentQueue);
}

void VkGraphicsWrapper::BindTextureBinding(TextureBinding *) {
	std::cout << "BindTextureBinding is not used in VkGraphicsWrapper\n";
	assert(true);
}

void VkGraphicsWrapper::BindVertexArrayObject(VertexArrayObject *) {
	std::cout << "BindVertexArrayObject is not used in VkGraphicsWrapper\n";
	assert(true);
}

void VkGraphicsWrapper::DrawImmediateIndexed(bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	std::cout << "DrawImmediateIndexed is not used in VkGraphicsWrapper\n";
	assert(true);
}

void VkGraphicsWrapper::DrawImmediateVertices(uint32_t base, uint32_t count)
{
}

void VkGraphicsWrapper::SetImmediateBlending(bool)
{
}

ColorFormat VkGraphicsWrapper::GetDeviceColorFormat() {
	switch (swapChainImageFormat) {
	case VK_FORMAT_R8_UNORM:
		return FORMAT_COLOR_R8;
	case VK_FORMAT_R8G8_UNORM:
		return FORMAT_COLOR_R8G8;
	case VK_FORMAT_B8G8R8_UINT:
		return FORMAT_COLOR_R8G8B8;
	case VK_FORMAT_R8G8B8A8_UNORM:
		return FORMAT_COLOR_R8G8B8A8;
	default:
		std::cout << "GetDeviceColorFormat returned incorrect color: " << swapChainImageFormat << "\n";
		return FORMAT_COLOR_R8G8B8A8;
	}
}

void VkGraphicsWrapper::SwapBuffer() {
	std::cout << "SwapBuffer is not used in VkGraphicsWrapper\n";
	assert(true);
}

void VkGraphicsWrapper::Cleanup() {
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	DestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroySurfaceKHR(instance, vkSurface, nullptr);

	vkDestroyInstance(instance, nullptr);
}

void VkGraphicsWrapper::DeleteFramebuffer(Framebuffer *ptr) {
	delete (vkFramebuffer *)ptr;
}

void VkGraphicsWrapper::DeleteVertexBuffer(VertexBuffer *ptr) {
	delete (vkVertexBuffer *)ptr;
}

void VkGraphicsWrapper::DeleteIndexBuffer(IndexBuffer *ptr) {
	delete (vkIndexBuffer *)ptr;
}

void VkGraphicsWrapper::DeleteUniformBuffer(UniformBuffer *ptr) {
	delete (vkUniformBuffer *)ptr;
}

void VkGraphicsWrapper::DeleteUniformBufferBinding(UniformBufferBinding * ptr) {
}

void VkGraphicsWrapper::DeleteGraphicsPipeline(GraphicsPipeline *ptr) {
	delete (vkGraphicsPipeline *)ptr;
}

void VkGraphicsWrapper::DeleteRenderPass(RenderPass *ptr) {
	delete (vkRenderPass *)ptr;
}

void VkGraphicsWrapper::DeleteTexture(Texture * ptr) {
	delete (vkTexture *)ptr;
}

void VkGraphicsWrapper::DeleteCommandBuffer(CommandBuffer *ptr) {
	delete (vkCommandBuffer *)ptr;
}

void VkGraphicsWrapper::DeleteVertexArrayObject(VertexArrayObject * ptr) {
}

Texture * VkGraphicsWrapper::CreateCubemap(CubemapCreateInfo createInfo) {
	return new vkTexture(&device, &physicalDevice, &commandPool, &descriptorPool, &graphicsQueue, createInfo);
}

TextureBindingLayout * VkGraphicsWrapper::CreateTextureBindingLayout(TextureBindingLayoutCreateInfo createInfo) {
	return new vkTextureBindingLayout(&device, createInfo);
}

Framebuffer * VkGraphicsWrapper::CreateFramebuffer(FramebufferCreateInfo ci) {
	return new vkFramebuffer(&device, &physicalDevice, ci);
}

RenderPass *VkGraphicsWrapper::CreateRenderPass(RenderPassCreateInfo ci) {
	return new vkRenderPass(&device, swapChainImageFormat, ci);
}

GraphicsPipeline * VkGraphicsWrapper::CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
	return new vkGraphicsPipeline(&device, ci);
}

void VkGraphicsWrapper::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

bool VkGraphicsWrapper::CheckValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void VkGraphicsWrapper::InitializeVulkan() {
	if (debug && !CheckValidationLayerSupport()) {
		throw std::runtime_error("VULKAN: Validation layers requested, but not available!");
	}

	// Vulkan Application Info
	VkApplicationInfo vkAppInfo{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// Type of Struct - Application Info
		nullptr,							// For extensions - 
		"Grind App",						// Application Name
		VK_MAKE_VERSION(1, 0, 0),			// Application Version
		"The Grind Engine",					// Engine Name
		VK_MAKE_VERSION(1, 0, 0),			// Engine Version
		VK_API_VERSION_1_0					// API version
	};

	// Vulkan Instance Info / Parameters
	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// Type of Struct - Instance
	createInfo.pNext = NULL;									// Next Instance
	createInfo.flags = 0;										// Flags
	createInfo.pApplicationInfo = &vkAppInfo;

	std::vector<const char *> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	if (debug) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	vkInitializeHandleResult(vkCreateInstance(&createInfo, NULL, &instance));
}

bool VkGraphicsWrapper::vkInitializeHandleResult(VkResult result) {
	const char *message;
	switch (result) {
	case VK_SUCCESS:
		message = "Vulkan has successfully initialized.\n";
		return true;
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		message = "Error: Vulkan cannot allocate enough memory to initialize.\n";
		return false;
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		message = "Error: Vulkan cannot allocate enough memory to initialize.\n";
		return false;
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		message = "Error: Vulkan cannot load one of its layers.\n";
		return false;
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		message = "Error: Vulkan cannot load one or more of its extensions.\n";
		return false;
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		message = "Error: Vulkan cannot find a compatible driver.\n";
		return false;
		break;
	default:
	case VK_ERROR_INITIALIZATION_FAILED:
		message = "Error: Vulkan failed to load for unspecified reasons.\n";
		return false;
		break;
	}

	fprintf(stderr, "VULKAN failed to Initialize.\n%s\n", message);
}

bool QueueFamilies::isComplete() {
	return graphics >= 0 && present >= 0;
}


GRAPHICS_EXPORT GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
	return new VkGraphicsWrapper(createInfo);
}

GRAPHICS_EXPORT void deleteGraphics(void * ptr) {
	free(ptr);
}