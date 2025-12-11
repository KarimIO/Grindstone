#include <cassert>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <set>
#include <algorithm>
#include <array>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanWindowGraphicsBinding.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCommandBuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanSampler.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanImage.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFramebuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanGraphicsPipeline.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanComputePipeline.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanVertexArrayObject.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanBuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSet.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSetLayout.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanUtils.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;
using namespace Grindstone::Memory;

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
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_GOOGLE_HLSL_FUNCTIONALITY_1_EXTENSION_NAME,
	VK_GOOGLE_USER_TYPE_EXTENSION_NAME,
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

constexpr auto vkApiVersion = VK_API_VERSION_1_3;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	Grindstone::GraphicsAPI::Vulkan::Core* vk = static_cast<Grindstone::GraphicsAPI::Vulkan::Core*>(pUserData);

	if (pCallbackData->messageIdNumber == 1402107823 || -507995293 == pCallbackData->messageIdNumber || 941228658 == pCallbackData->messageIdNumber || pCallbackData->messageIdNumber == 941228658) {
		return VK_FALSE;
	}

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

	GPRINT_TYPED(logSeverity, Grindstone::LogSource::GraphicsAPI, pCallbackData->messageIdNumber, pCallbackData->pMessage);

	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

Vulkan::Core *Vulkan::Core::graphicsWrapper = nullptr;

bool Vulkan::Core::Initialize(const Base::Core::CreateInfo& ci) {
	apiType = API::Vulkan;
	graphicsWrapper = this;
	debug = ci.debug;
	primaryWindow = ci.window;

	CreateInstance();
	auto wgb = AllocatorCore::Allocate<Vulkan::WindowGraphicsBinding>();
	primaryWindow->AddBinding(wgb);
	wgb->Initialize(primaryWindow);
	if (ci.debug) {
		SetupDebugMessenger();
	}

	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateAllocator();

	pfnDebugUtilsSetObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
	Vulkan::CommandBuffer::SetupDebugLabelUtils(instance);

	if (primaryWindow->IsSwapchainControlledByEngine()) {
		wgb->CreateSwapChain();
	}
	wgb->CreateSyncObjects();
	CreateCommandPool();
	CreateDescriptorPool();

	return true;
}

void Vulkan::Core::CreateAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = vkApiVersion;
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

	vmaCreateAllocator(&allocatorInfo, &allocator);
}

void Vulkan::Core::CreateInstance() {
	bool areValidationLayersFound = CheckValidationLayerSupport();
	if (enableValidationLayers && !areValidationLayersFound) {
		GPRINT_ERROR(LogSource::GraphicsAPI, "Vulkan validation support is requested, but not available. Please install https://vulkan.lunarg.com/");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Grindstone";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Grindstone Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = vkApiVersion;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();
	extensions.push_back(VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers && areValidationLayersFound) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create instance!");
	}
}

void Vulkan::Core::SetupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to set up debug messenger!");
	}
}

void Vulkan::Core::PickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to find GPUs with Vulkan:: support!");
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
		GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan: Failed to find a suitable GPU!");
	}

	VkPhysicalDeviceProperties gpuProperties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &gpuProperties);

	GPRINT_INFO_V(LogSource::GraphicsAPI, "Using Device: {}", gpuProperties.deviceName);

	const char *vendorName = GetVendorNameFromID(gpuProperties.vendorID);
	if (vendorName == nullptr) {
		this->vendorName = std::string("Unknown Vendor(") + std::to_string(gpuProperties.vendorID) + ")";
	}
	else {
		this->vendorName = vendorName;
	}
	adapterName = gpuProperties.deviceName;

	unsigned int versionMajor = (gpuProperties.apiVersion >> 22) & 0x3FF;
	unsigned int versionMinor = (gpuProperties.apiVersion >> 12) & 0x3FF;
	unsigned int versionPatch = (gpuProperties.apiVersion) & 0xfff;
	apiVersion = std::to_string(versionMajor)+"."+ std::to_string(versionMinor) + "." + std::to_string(versionPatch);

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
	graphicsFamily = indices.graphicsFamily;
	presentFamily = indices.presentFamily;
}

void Vulkan::Core::CreateLogicalDevice() {
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

	VkPhysicalDeviceVulkan13Features deviceFeatures13 = {};
	deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	deviceFeatures13.dynamicRendering = VK_TRUE;

	VkPhysicalDeviceVulkan12Features deviceFeatures12 = {};
	deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	deviceFeatures12.pNext = &deviceFeatures13;

	VkPhysicalDeviceVulkan11Features deviceFeatures11 = {};
	deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	deviceFeatures11.shaderDrawParameters = VK_TRUE;
	deviceFeatures11.pNext = &deviceFeatures12;

	VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.features.fragmentStoresAndAtomics = VK_TRUE;
	deviceFeatures2.features.fillModeNonSolid = VK_TRUE;
	deviceFeatures2.features.multiDrawIndirect = VK_TRUE;
	deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures2.pNext = &deviceFeatures11;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pNext = &deviceFeatures2;

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
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create logical device!");
	}

	vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
}

void Vulkan::Core::CreateCommandPool() {
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = graphicsFamily;

	if (vkCreateCommandPool(device, &poolCreateInfo, allocator->GetAllocationCallbacks(), &commandPoolGraphics) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create graphics command pool!");
	}
}

bool Vulkan::Core::CheckValidationLayerSupport() {
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

Vulkan::QueueFamilyIndices Vulkan::Core::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
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

std::vector<const char*> Vulkan::Core::GetRequiredExtensions() {
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

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void Vulkan::Core::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = this;
}

uint16_t Vulkan::Core::ScoreDevice(VkPhysicalDevice device) {
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		auto wgb = static_cast<Vulkan::WindowGraphicsBinding*>(primaryWindow->GetWindowGraphicsBinding());
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

bool Vulkan::Core::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
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

Vulkan::Core::~Core() {
	for (auto& pipeline : graphicsPipelineCache) {
		DeleteGraphicsPipeline(pipeline.second);
	}

	vkDestroyCommandPool(device, commandPoolGraphics, allocator->GetAllocationCallbacks());
	vkDestroyDescriptorPool(device, descriptorPool, allocator->GetAllocationCallbacks());

	vkDestroyDevice(device, allocator->GetAllocationCallbacks());

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocator->GetAllocationCallbacks());
	}

	vkDestroyInstance(instance, allocator->GetAllocationCallbacks());

}

void Vulkan::Core::WaitUntilIdle() {
	vkDeviceWaitIdle(device);
}

void Vulkan::Core::CreateDescriptorPool() {
	std::array<VkDescriptorPoolSize, 6> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1000;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = 1000;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = 1000;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[3].descriptorCount = 1000;
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSizes[4].descriptorCount = 1000;
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSizes[5].descriptorCount = 1000;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(poolSizes.size() * 1000);

	if (vkCreateDescriptorPool(device, &poolInfo, allocator->GetAllocationCallbacks(), &descriptorPool) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create descriptor pool!");
	}
}

Vulkan::Core &Vulkan::Core::Get() {
	return *graphicsWrapper;
}

void Vulkan::Core::RegisterWindow(Window* window) {
	auto wgb = AllocatorCore::Allocate<Vulkan::WindowGraphicsBinding>();
	window->AddBinding(wgb);
	wgb->Initialize(window);
	//wgb->shareLists((VulkanWindowGraphicsBinding*)primary_window_->getWindowGraphicsBinding());
}

uint32_t Vulkan::Core::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	GPRINT_FATAL(LogSource::GraphicsAPI, "failed to find suitable memory type!");

	return UINT32_MAX;
}

VkInstance Vulkan::Core::GetInstance() {
	return instance;
}

VkDevice Vulkan::Core::GetDevice() {
	return device;
}

VkPhysicalDevice Vulkan::Core::GetPhysicalDevice() {
	return physicalDevice;
}

VkCommandBuffer Vulkan::Core::BeginSingleTimeCommands() {
	return Vulkan::BeginSingleTimeCommands();
}

uint32_t Vulkan::Core::GetGraphicsFamily() {
	return graphicsFamily;
}

void Vulkan::Core::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	Vulkan::EndSingleTimeCommands(commandBuffer);
}

VkCommandPool Vulkan::Core::GetGraphicsCommandPool() const {
	return commandPoolGraphics;
}

void Vulkan::Core::AdjustPerspective(float *perspective) {
	perspective[1*4 + 1] *= -1;
}

void Vulkan::Core::NameObject(
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
const char* Vulkan::Core::GetVendorName() const {
	return vendorName.c_str();
}

const char* Vulkan::Core::GetAdapterName() const {
	return adapterName.c_str();
}

const char* Vulkan::Core::GetAPIName() const {
	return "Vulkan";
}

const char* Vulkan::Core::GetAPIVersion() const {
	return apiVersion.c_str();
}

const char* Vulkan::Core::GetDefaultShaderExtension() const {
	return ".vk.spv";
}

//==================================
// Creators
//==================================
Base::Framebuffer* Vulkan::Core::CreateFramebuffer(const Base::Framebuffer::CreateInfo& ci) {
	return static_cast<Base::Framebuffer*>(AllocatorCore::AllocateNamed<Vulkan::Framebuffer>(ci.debugName ? ci.debugName : "Vulkan::Framebuffer", ci));
}

Base::RenderPass* Vulkan::Core::CreateRenderPass(const Base::RenderPass::CreateInfo& ci) {
	return static_cast<Base::RenderPass*>(AllocatorCore::AllocateNamed<Vulkan::RenderPass>(ci.debugName ? ci.debugName : "Vulkan::RenderPass", ci));
}

Base::ComputePipeline* Vulkan::Core::CreateComputePipeline(const Base::ComputePipeline::CreateInfo& ci) {
	return static_cast<Base::ComputePipeline*>(AllocatorCore::AllocateNamed<Vulkan::ComputePipeline>(ci.debugName ? ci.debugName : "Vulkan::ComputePipeline", ci));
}

Base::GraphicsPipeline* Vulkan::Core::CreateGraphicsPipeline(const Base::GraphicsPipeline::CreateInfo& ci) {
	return static_cast<Base::GraphicsPipeline*>(AllocatorCore::AllocateNamed<Vulkan::GraphicsPipeline>(ci.pipelineData.debugName ? ci.pipelineData.debugName : "Vulkan::GraphicsPipeline", ci));
}

Base::CommandBuffer* Vulkan::Core::CreateCommandBuffer(const Base::CommandBuffer::CreateInfo& ci) {
	return static_cast<Base::CommandBuffer*>(AllocatorCore::AllocateNamed<Vulkan::CommandBuffer>(ci.debugName ? ci.debugName : "Vulkan::CommandBuffer", ci));
}

Base::VertexArrayObject* Vulkan::Core::CreateVertexArrayObject(const Base::VertexArrayObject::CreateInfo& ci) {
	return static_cast<Base::VertexArrayObject*>(AllocatorCore::AllocateNamed<Vulkan::VertexArrayObject>(ci.debugName ? ci.debugName : "Vulkan::VertexArrayObject", ci));
}

Base::Buffer* Vulkan::Core::CreateBuffer(const Base::Buffer::CreateInfo& ci) {
	return static_cast<Base::Buffer*>(AllocatorCore::AllocateNamed<Vulkan::Buffer>(ci.debugName ? ci.debugName : "Vulkan::Buffer", ci));
}

Base::Sampler* Vulkan::Core::CreateSampler(const Base::Sampler::CreateInfo& createInfo) {
	return static_cast<Base::Sampler*>(AllocatorCore::AllocateNamed<Vulkan::Sampler>(createInfo.debugName ? createInfo.debugName : "Vulkan::Sampler", createInfo));
}

Base::Image* Vulkan::Core::CreateImage(const Base::Image::CreateInfo& createInfo) {
	return static_cast<Base::Image*>(AllocatorCore::AllocateNamed<Vulkan::Image>(createInfo.debugName ? createInfo.debugName : "Vulkan::Image", createInfo));
}

Base::DescriptorSet* Vulkan::Core::CreateDescriptorSet(const Base::DescriptorSet::CreateInfo& ci) {
	return static_cast<Base::DescriptorSet*>(AllocatorCore::AllocateNamed<Vulkan::DescriptorSet>(ci.debugName ? ci.debugName : "Vulkan::DescriptorSet", ci));
}

Base::DescriptorSetLayout* Vulkan::Core::CreateDescriptorSetLayout(const Base::DescriptorSetLayout::CreateInfo& ci) {
	return static_cast<Base::DescriptorSetLayout*>(AllocatorCore::AllocateNamed<Vulkan::DescriptorSetLayout>(ci.debugName ? ci.debugName : "Vulkan::DescriptorSetLayout", ci));
}

Base::GraphicsPipeline* Vulkan::Core::GetOrCreateGraphicsPipelineFromCache(
	const GraphicsPipeline::PipelineData& pipelineData,
	const VertexInputLayout* vertexInputLayout
) {
	size_t hash = std::hash<GraphicsPipeline::PipelineData>{}(pipelineData);
	if (vertexInputLayout != nullptr) {
		hash ^= std::hash<VertexInputLayout>{}(*vertexInputLayout);
	}
	auto iterator = graphicsPipelineCache.find(hash);
	if (iterator != graphicsPipelineCache.end()) {
		return iterator->second;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline::CreateInfo createInfo{};
	createInfo.pipelineData = pipelineData;
	if (vertexInputLayout != nullptr) {
		createInfo.vertexInputLayout = *vertexInputLayout;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* newPipeline = CreateGraphicsPipeline(createInfo);

	graphicsPipelineCache[hash] = newPipeline;
	return newPipeline;
}

// Deleters
//==================================
void Vulkan::Core::DeleteFramebuffer(Base::Framebuffer *ptr) {
	AllocatorCore::Free(static_cast<Vulkan::Framebuffer*>(ptr));
}

void Vulkan::Core::DeleteVertexArrayObject(Base::VertexArrayObject* ptr) {
	AllocatorCore::Free(static_cast<Vulkan::VertexArrayObject*>(ptr));
}

void Vulkan::Core::DeleteBuffer(Base::Buffer *ptr) {
	AllocatorCore::Free(static_cast<Vulkan::Buffer*>(ptr));
}

void Vulkan::Core::DeleteComputePipeline(Base::ComputePipeline* ptr) {
	AllocatorCore::Free(static_cast<Vulkan::ComputePipeline*>(ptr));
}

void Vulkan::Core::DeleteGraphicsPipeline(Base::GraphicsPipeline *ptr) {
	AllocatorCore::Free(static_cast<Vulkan::GraphicsPipeline*>(ptr));
}

void Vulkan::Core::DeleteRenderPass(Base::RenderPass *ptr) {
	AllocatorCore::Free(static_cast<Vulkan::RenderPass*>(ptr));
}

void Vulkan::Core::DeleteSampler(Base::Sampler* ptr) {
	AllocatorCore::Free(static_cast<Vulkan::Sampler*>(ptr));
}

void Vulkan::Core::DeleteImage(Base::Image* ptr) {
	AllocatorCore::Free(static_cast<Vulkan::Image*>(ptr));
}

void Vulkan::Core::DeleteDescriptorSet(Base::DescriptorSet* ptr) {
	AllocatorCore::Free(static_cast<Vulkan::DescriptorSet*>(ptr));
}

void Vulkan::Core::DeleteDescriptorSetLayout(Base::DescriptorSetLayout * ptr) {
	AllocatorCore::Free(static_cast<Vulkan::DescriptorSetLayout*>(ptr));
}

void Vulkan::Core::DeleteCommandBuffer(Base::CommandBuffer *ptr) {
	AllocatorCore::Free(static_cast<Vulkan::CommandBuffer*>(ptr));
}

//==================================
// Booleans
//==================================
inline bool Vulkan::Core::ShouldUseImmediateMode() const {
	return false;
}
inline bool Vulkan::Core::SupportsCommandBuffers() const {
	return false;
}
inline bool Vulkan::Core::SupportsTesselation() const {
	return false;
}
inline bool Vulkan::Core::SupportsGeometryShader() const {
	return false;
}
inline bool Vulkan::Core::SupportsComputeShader() const {
	return false;
}
inline bool Vulkan::Core::SupportsMultiDrawIndirect() const {
	return false;
}

//==================================
// Unused
//==================================
void Vulkan::Core::Clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::Clear is not used.");
	assert(false);
}
void Vulkan::Core::BindGraphicsPipeline(Base::GraphicsPipeline*) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::BindGraphicsPipeline is not used.");
	assert(false);
}
void Vulkan::Core::BindVertexArrayObject(Base::VertexArrayObject *) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::BindVertexArrayObject is not used.");
	assert(false);
}
void Vulkan::Core::DrawImmediateIndexed(GeometryType geometryType, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::DrawImmediateIndexed is not used.");
	assert(false);
}
void Vulkan::Core::DrawImmediateVertices(GeometryType geometryType, uint32_t base, uint32_t count) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::DrawImmediateVertices is not used.");
	assert(false);
}
void Vulkan::Core::SetImmediateBlending(
	BlendOperation colorOp, BlendFactor colorSrc, BlendFactor colorDst,
	BlendOperation alphaOp, BlendFactor alphaSrc, BlendFactor alphaDst
) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::SetImmediateBlending is not used.");
	assert(false);
}
void Vulkan::Core::EnableDepthWrite(bool state) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::EnableDepthWrite is not used.");
	assert(false);
}
void Vulkan::Core::SetColorMask(ColorMask mask) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::Core::SetColorMask is not used.");
	assert(false);
}

void Vulkan::Core::CopyDepthBufferFromReadToWrite(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) {
}

void Vulkan::Core::BindDefaultFramebuffer() {
}

void Vulkan::Core::BindDefaultFramebufferWrite() {
}

void Vulkan::Core::BindDefaultFramebufferRead() {
}

void Vulkan::Core::ResizeViewport(uint32_t w, uint32_t h) {
}
