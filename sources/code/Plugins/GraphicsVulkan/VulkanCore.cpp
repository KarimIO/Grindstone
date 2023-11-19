#define NOMINMAX

#include <cassert>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "VulkanWindowGraphicsBinding.hpp"
#include "VulkanCore.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanComputePipeline.hpp"
#include "VulkanIndexBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanTexture.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanVertexArrayObject.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUtils.hpp"
#include <set>
#include <algorithm>
#include <array>

using namespace Grindstone::GraphicsAPI;

const int MAX_FRAMES_IN_FLIGHT = 2;

const float B_IN_KB = 1024;
const float KB_IN_MB = 1024;
const float MB_IN_GB = 1024;
const float HEAP_SIZE_IN_GB_MULTIPLIER = 1.0f / B_IN_KB / KB_IN_MB / MB_IN_GB;

const size_t DISCRETE_GPU_BONUS = 200;
const size_t INTEGRATED_GPU_BONUS = 100;
const float HEAP_SCORE_MULTIPLIER = 10.0f;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	Grindstone::GraphicsAPI::VulkanCore* vk = static_cast<Grindstone::GraphicsAPI::VulkanCore*>(pUserData);

	Grindstone::LogSeverity logSeverity = Grindstone::LogSeverity::Info;
	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		logSeverity = Grindstone::LogSeverity::Error;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		logSeverity = Grindstone::LogSeverity::Warning;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		logSeverity = Grindstone::LogSeverity::Info;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		logSeverity = Grindstone::LogSeverity::Trace;
		break;
	}

	vk->logFunction(logSeverity, pCallbackData->pMessage);

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VulkanCore *VulkanCore::graphicsWrapper = nullptr;

VulkanCore::VulkanCore(std::function<void(LogSeverity, const char*)> logFunction) : logFunction(logFunction) {}

bool VulkanCore::Initialize(Core::CreateInfo& ci) {
	apiType = API::Vulkan;
	graphicsWrapper = this;
	debug = ci.debug;
	primaryWindow = ci.window;

	CreateInstance();
	auto wgb = new VulkanWindowGraphicsBinding();
	primaryWindow->AddBinding(wgb);
	wgb->Initialize(primaryWindow);
	if (ci.debug) {
		SetupDebugMessenger();
	}

	PickPhysicalDevice();
	CreateLogicalDevice();

	pfnDebugUtilsSetObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));

	if (primaryWindow->IsSwapchainControlledByEngine()) {
		wgb->CreateSwapChain();
	}
	wgb->CreateSyncObjects();
	CreateCommandPool();
	CreateDescriptorPool();

	return true;
}

void VulkanCore::CreateInstance() {
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Grindstone";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Grindstone Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();
	extensions.push_back(VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanCore::SetupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void VulkanCore::PickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	uint16_t scoreMax = 0;

	for (const auto& device : devices) {
		uint16_t score = ScoreDevice(device);
		if (score > scoreMax) {
			physicalDevice = device;
			scoreMax = score;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Vulkan: Failed to find a suitable GPU!");
	}

	VkPhysicalDeviceProperties gpuProps{};
	vkGetPhysicalDeviceProperties(physicalDevice, &gpuProps);

	std::string output = std::string("Using Device: ") + gpuProps.deviceName;
	logFunction(LogSeverity::Info, output.c_str());

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	const char *vendorName = GetVendorNameFromID(properties.vendorID);
	if (vendorName == nullptr) {
		this->vendorName = std::string("Unknown Vendor(") + std::to_string(properties.vendorID) + ")";
	}
	else {
		this->vendorName = vendorName;
	}
	adapterName = properties.deviceName;

	unsigned int versionMajor = (properties.apiVersion >> 22) & 0x3FF;
	unsigned int versionMinor = (properties.apiVersion >> 12) & 0x3FF;
	unsigned int versionPatch = (properties.apiVersion) & 0xfff;
	apiVersion = std::to_string(versionMajor)+"."+ std::to_string(versionMinor) + "." + std::to_string(versionPatch);

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
	graphicsFamily = indices.graphicsFamily;
	presentFamily = indices.presentFamily;
}

void VulkanCore::CreateLogicalDevice() {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily, presentFamily };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
}

void VulkanCore::CreateCommandPool() {
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = graphicsFamily;

	if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPoolGraphics) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

bool VulkanCore::CheckValidationLayerSupport() {
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

QueueFamilyIndices VulkanCore::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			indices.hasGraphicsFamily = true;
		}

		if (glfwGetPhysicalDevicePresentationSupport(instance, device, i)) {
			indices.presentFamily = i;
			indices.hasPresentFamily = true;
		}

		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

std::vector<const char*> VulkanCore::GetRequiredExtensions() {
	std::vector<const char*> extensions;
	/* TODO: This was removed to switch to glfw. When supporting Win32 or Xlib again, reimplement this.
	= {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
	};
	*/

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	uint32_t extensionCount = enableValidationLayers
		? glfwExtensionCount + 1
		: glfwExtensionCount;

	extensions.reserve(extensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanCore::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = this;
}

uint16_t VulkanCore::ScoreDevice(VkPhysicalDevice device) {
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		auto wgb = static_cast<VulkanWindowGraphicsBinding*>(primaryWindow->GetWindowGraphicsBinding());
		SwapChainSupportDetails swapChainSupport = wgb->QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	if (!indices.IsComplete() || !extensionsSupported || !swapChainAdequate) {
		return 0;
	}

	VkPhysicalDeviceProperties gpuProps{};
	vkGetPhysicalDeviceProperties(device, &gpuProps);

	size_t gpuTypeScore = 0;
	if (gpuProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		gpuTypeScore += DISCRETE_GPU_BONUS;
	}
	else if (gpuProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
		gpuTypeScore += INTEGRATED_GPU_BONUS;
	}

	VkPhysicalDeviceMemoryProperties memoryProps{};
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProps);

	auto heapsPointer = memoryProps.memoryHeaps;
	auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memoryProps.memoryHeapCount);

	float heapScore = 0;
	for (const auto& heap : heaps) {
		if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			heapScore += heap.size * HEAP_SIZE_IN_GB_MULTIPLIER * HEAP_SCORE_MULTIPLIER;
			break;
		}
	}

	return static_cast<uint16_t>(heapScore + gpuTypeScore);
}

bool VulkanCore::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VulkanCore::~VulkanCore() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPoolGraphics, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyInstance(instance, nullptr);

}

void VulkanCore::WaitUntilIdle() {
	vkDeviceWaitIdle(device);
}

void VulkanCore::CreateDescriptorPool() {
	std::array<VkDescriptorPoolSize, 3> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1000; // static_cast<uint32_t>(swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1000; //static_cast<uint32_t>(swapChainImages.size());
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[2].descriptorCount = 1000; //static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 3000;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

VulkanCore &VulkanCore::Get() {
	return *graphicsWrapper;
}

void VulkanCore::RegisterWindow(Window* window) {
	auto wgb = new VulkanWindowGraphicsBinding();
	window->AddBinding(wgb);
	wgb->Initialize(window);
	//wgb->shareLists((VulkanWindowGraphicsBinding*)primary_window_->getWindowGraphicsBinding());
}

uint32_t VulkanCore::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

VkInstance VulkanCore::GetInstance() {
	return instance;
}

VkDevice VulkanCore::GetDevice() {
	return device;
}

VkPhysicalDevice VulkanCore::GetPhysicalDevice() {
	return physicalDevice;
}

VkCommandBuffer VulkanCore::BeginSingleTimeCommands() {
	return GraphicsAPI::BeginSingleTimeCommands();
}

uint32_t VulkanCore::GetGraphicsFamily() {
	return graphicsFamily;
}

void VulkanCore::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	GraphicsAPI::EndSingleTimeCommands(commandBuffer);
}

VkCommandPool VulkanCore::GetGraphicsCommandPool() {
	return commandPoolGraphics;
}

void VulkanCore::AdjustPerspective(float *perspective) {
	perspective[1*4 + 1] *= -1;
}

void VulkanCore::NameObject(
	VkObjectType objectType,
	void* object,
	const char* objectName
) {
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = objectType;
	nameInfo.objectHandle = (uint64_t)object;
	nameInfo.pObjectName = objectName;
	pfnDebugUtilsSetObjectName(device, &nameInfo);
}

//==================================
// Get Text Metainfo
//==================================
const char* VulkanCore::GetVendorName() {
	return vendorName.c_str();
}

const char* VulkanCore::GetAdapterName() {
	return adapterName.c_str();
}

const char* VulkanCore::GetAPIName() {
	return "Vulkan";
}

const char* VulkanCore::GetAPIVersion() {
	return apiVersion.c_str();
}

const char* VulkanCore::GetDefaultShaderExtension() {
	return ".vk.spv";
}

//==================================
// Creators
//==================================
Framebuffer *VulkanCore::CreateFramebuffer(Framebuffer::CreateInfo& ci) {
	return static_cast<Framebuffer*>(new VulkanFramebuffer(ci));
}

RenderPass * VulkanCore::CreateRenderPass(RenderPass::CreateInfo& ci) {
	return static_cast<RenderPass*>(new VulkanRenderPass(ci));
}

ComputePipeline* VulkanCore::CreateComputePipeline(ComputePipeline::CreateInfo& ci) {
	return static_cast<ComputePipeline*>(new VulkanComputePipeline(ci));
}

GraphicsPipeline* VulkanCore::CreateGraphicsPipeline(GraphicsPipeline::CreateInfo& ci) {
	return static_cast<GraphicsPipeline*>(new VulkanGraphicsPipeline(ci));
}

CommandBuffer * VulkanCore::CreateCommandBuffer(CommandBuffer::CreateInfo& ci) {
	return static_cast<CommandBuffer*>(new VulkanCommandBuffer(ci));
}

VertexArrayObject* VulkanCore::CreateVertexArrayObject(VertexArrayObject::CreateInfo& ci) {
	return static_cast<VertexArrayObject*>(new VulkanVertexArrayObject(ci));
}

VertexBuffer * VulkanCore::CreateVertexBuffer(VertexBuffer::CreateInfo& ci) {
	return static_cast<VertexBuffer*>(new VulkanVertexBuffer(ci));
}

IndexBuffer * VulkanCore::CreateIndexBuffer(IndexBuffer::CreateInfo& ci) {
	return static_cast<IndexBuffer*>(new VulkanIndexBuffer(ci));
}

UniformBuffer * VulkanCore::CreateUniformBuffer(UniformBuffer::CreateInfo& ci) {
	return static_cast<UniformBuffer*>(new VulkanUniformBuffer(ci));
}

Texture* VulkanCore::CreateCubemap(Texture::CubemapCreateInfo& createInfo) {
	return nullptr; // static_cast<Texture*>(new VulkanTexture(ci));
}

Texture* VulkanCore::CreateTexture(Texture::CreateInfo& ci) {
	return static_cast<Texture*>(new VulkanTexture(ci));
}

DescriptorSet* VulkanCore::CreateDescriptorSet(DescriptorSet::CreateInfo& ci) {
	return static_cast<DescriptorSet*>(new VulkanDescriptorSet(ci));
}

DescriptorSetLayout* VulkanCore::CreateDescriptorSetLayout(DescriptorSetLayout::CreateInfo& ci) {
	return static_cast<DescriptorSetLayout*>(new VulkanDescriptorSetLayout(ci));
}

RenderTarget* VulkanCore::CreateRenderTarget(RenderTarget::CreateInfo& ci) {
	return static_cast<RenderTarget*>(new VulkanRenderTarget(ci));
}

RenderTarget* VulkanCore::CreateRenderTarget(RenderTarget::CreateInfo* ci, uint32_t rc, bool cube) {
	return static_cast<RenderTarget*>(new VulkanRenderTarget(*ci));
}

DepthTarget * VulkanCore::CreateDepthTarget(DepthTarget::CreateInfo& ci) {
	return static_cast<DepthTarget*>(new VulkanDepthTarget(ci));
}

//==================================
// Deleters
//==================================
void VulkanCore::DeleteRenderTarget(RenderTarget * ptr) {
	delete static_cast<VulkanRenderTarget*>(ptr);
}
void VulkanCore::DeleteDepthTarget(DepthTarget * ptr) {
	delete static_cast<VulkanDepthTarget*>(ptr);
}
void VulkanCore::DeleteFramebuffer(Framebuffer *ptr) {
	delete static_cast<VulkanFramebuffer*>(ptr);
}
void VulkanCore::DeleteVertexArrayObject(VertexArrayObject* ptr) {
	delete static_cast<VulkanVertexArrayObject*>(ptr);
}
void VulkanCore::DeleteVertexBuffer(VertexBuffer *ptr) {
	delete static_cast<VulkanVertexBuffer*>(ptr);
}
void VulkanCore::DeleteIndexBuffer(IndexBuffer *ptr) {
	delete static_cast<VulkanIndexBuffer*>(ptr);
}
void VulkanCore::DeleteUniformBuffer(UniformBuffer *ptr) {
	delete static_cast<VulkanUniformBuffer*>(ptr);
}
void VulkanCore::DeleteComputePipeline(ComputePipeline* ptr) {
	delete static_cast<VulkanComputePipeline*>(ptr);
}
void VulkanCore::DeleteGraphicsPipeline(GraphicsPipeline *ptr) {
	delete static_cast<VulkanGraphicsPipeline*>(ptr);
}
void VulkanCore::DeleteRenderPass(RenderPass *ptr) {
	delete static_cast<VulkanRenderPass*>(ptr);
}
void VulkanCore::DeleteTexture(Texture * ptr) {
	delete static_cast<VulkanTexture*>(ptr);
}
void VulkanCore::DeleteDescriptorSet(DescriptorSet* ptr) {
	delete static_cast<VulkanDescriptorSet*>(ptr);
}
void VulkanCore::DeleteDescriptorSetLayout(DescriptorSetLayout * ptr) {
	delete static_cast<VulkanDescriptorSetLayout*>(ptr);
}
void VulkanCore::DeleteCommandBuffer(CommandBuffer *ptr) {
	delete static_cast<VulkanCommandBuffer*>(ptr);
}

//==================================
// Booleans
//==================================
inline const bool VulkanCore::ShouldUseImmediateMode() {
	return false;
}
inline const bool VulkanCore::SupportsCommandBuffers() {
	return false;
}
inline const bool VulkanCore::SupportsTesselation() {
	return false;
}
inline const bool VulkanCore::SupportsGeometryShader() {
	return false;
}
inline const bool VulkanCore::SupportsComputeShader() {
	return false;
}
inline const bool VulkanCore::SupportsMultiDrawIndirect() {
	return false;
}

//==================================
// Unused
//==================================
void VulkanCore::Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
	std::cout << "VulkanCore::Clear is not used.\n";
	assert(false);
}
void VulkanCore::BindGraphicsPipeline(GraphicsPipeline*) {
	std::cout << "VulkanCore::BindGraphicsPipeline is not used.\n";
	assert(false);
}
void VulkanCore::BindVertexArrayObject(VertexArrayObject *) {
	std::cout << "VulkanCore::BindVertexArrayObject is not used.\n";
	assert(false);
}
void VulkanCore::DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	std::cout << "VulkanCore::DrawImmediateIndexed is not used.\n";
	assert(false);
}
void VulkanCore::DrawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) {
	std::cout << "VulkanCore::DrawImmediateVertices is not used.\n";
	assert(false);
}
void VulkanCore::SetImmediateBlending(BlendMode) {
	std::cout << "VulkanCore::SetImmediateBlending is not used.\n";
	assert(false);
}
void VulkanCore::EnableDepthWrite(bool state) {
	std::cout << "VulkanCore::EnableDepthWrite is not used.\n";
	assert(false);
}
void VulkanCore::SetColorMask(ColorMask mask) {
	std::cout << "VulkanCore::SetColorMask is not used.\n";
	assert(false);
}

void VulkanCore::CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) {
}

void VulkanCore::BindDefaultFramebuffer() {
}

void VulkanCore::BindDefaultFramebufferWrite() {
}

void VulkanCore::BindDefaultFramebufferRead() {
}

void VulkanCore::ResizeViewport(uint32_t w, uint32_t h) {
}
