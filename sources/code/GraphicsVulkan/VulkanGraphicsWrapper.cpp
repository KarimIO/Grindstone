#define NOMINMAX

#include <cassert>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

#include "VulkanGraphicsWrapper.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanDescriptors.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanIndexBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanTexture.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanFormat.hpp"
#include <set>
#include <algorithm>
#include <array>

const int MAX_FRAMES_IN_FLIGHT = 2;

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
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

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


namespace Grindstone {
	namespace GraphicsAPI {
		VulkanGraphicsWrapper *VulkanGraphicsWrapper::graphics_wrapper_ = nullptr;

		VulkanGraphicsWrapper::VulkanGraphicsWrapper(InstanceCreateInfo ci) {
			graphics_wrapper_ = this;
			debug_ = ci.debug;
			vsync_ = ci.vsync;
			width_ = ci.width;
			height_ = ci.height;
			title_ = ci.title;
			input_ = ci.inputInterface;

			createWindow();
			createInstance();
			if (ci.debug)
				setupDebugMessenger();
			createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
			createSwapChain();
			createCommandPool();
			createSyncObjects();
			createDescriptorPool();
		}

		void VulkanGraphicsWrapper::createWindow() {
			InitializeWin32Window();
		}
		void VulkanGraphicsWrapper::createInstance() {
			if (enableValidationLayers && !checkValidationLayerSupport()) {
				throw std::runtime_error("validation layers requested, but not available!");
			}

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Triangle";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			auto extensions = getRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();

				populateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else {
				createInfo.enabledLayerCount = 0;

				createInfo.pNext = nullptr;
			}

			if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create instance!");
			}
		}

		void VulkanGraphicsWrapper::setupDebugMessenger() {
			if (!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			populateDebugMessengerCreateInfo(createInfo);

			if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debug_messenger_) != VK_SUCCESS) {
				throw std::runtime_error("failed to set up debug messenger!");
			}
		}

		void VulkanGraphicsWrapper::createSurface() {
#ifdef VK_USE_PLATFORM_WIN32_KHR
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.hwnd = window_handle;
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateWin32SurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS)
				return;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.dpy = xDisplay;
			surfaceCreateInfo.window = xWindow;
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateXlibSurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS)
				return;
#endif
		}

		void VulkanGraphicsWrapper::pickPhysicalDevice() {
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

			if (deviceCount == 0) {
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

			int scoreMax = 0;

			std::cout << "Available Devices:\r\n";
			for (const auto& device : devices) {
				int score = scoreDevice(device);
				if (score > scoreMax) {
					physical_device_ = device;
				}
				break;
			}

			QueueFamilyIndices indices = findQueueFamilies(physical_device_);
			graphics_family_ = indices.graphicsFamily;
			present_family_ = indices.presentFamily;

			if (physical_device_ == VK_NULL_HANDLE) {
				throw std::runtime_error("failed to find a suitable GPU!");
			}
		}

		void VulkanGraphicsWrapper::createLogicalDevice() {
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { graphics_family_, present_family_ };

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

			if (vkCreateDevice(physical_device_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create logical device!");
			}

			vkGetDeviceQueue(device_, graphics_family_, 0, &graphics_queue_);
			vkGetDeviceQueue(device_, present_family_, 0, &present_queue_);
		}

		VkSurfaceFormatKHR VulkanGraphicsWrapper::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		VkPresentModeKHR VulkanGraphicsWrapper::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D VulkanGraphicsWrapper::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != UINT32_MAX) {
				return capabilities.currentExtent;
			}
			else {

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width_),
					static_cast<uint32_t>(height_)
				};

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		SwapChainSupportDetails VulkanGraphicsWrapper::querySwapChainSupport(VkPhysicalDevice device) {
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		void VulkanGraphicsWrapper::createSwapChain() {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device_);

			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface_;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = findQueueFamilies(physical_device_);
			uint32_t queueFamilyIndices[] = { graphics_family_, present_family_ };

			if (indices.graphicsFamily != indices.presentFamily) {
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swap_chain_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}

			std::vector<VkImage> swap_chain_images_;
			vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, nullptr);
			swap_chain_images_.resize(imageCount);
			swap_chain_targets_.resize(imageCount);
			vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, swap_chain_images_.data());

			for (uint32_t i = 0; i < imageCount; ++i) {
				swap_chain_targets_[i] = new VulkanRenderTarget(swap_chain_images_[i], surfaceFormat.format);
			}

			swapchain_format_ = TranslateColorFormatFromVulkan(surfaceFormat.format);
		}

		void VulkanGraphicsWrapper::createCommandPool() {
			VkCommandPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			pool_info.queueFamilyIndex = graphics_family_;

			if (vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_graphics_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics command pool!");
			}
		}

		bool VulkanGraphicsWrapper::checkValidationLayerSupport() {
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

		QueueFamilyIndices VulkanGraphicsWrapper::findQueueFamilies(VkPhysicalDevice device) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
				}

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

				if (presentSupport) {
					indices.presentFamily = i;
				}

				if (indices.isComplete()) {
					break;
				}

				i++;
			}

			return indices;
		}

		std::vector<const char*> VulkanGraphicsWrapper::getRequiredExtensions() {
			std::vector<const char *> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
			};

			if (enableValidationLayers) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensions;
		}

		void VulkanGraphicsWrapper::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
		}

		uint16_t VulkanGraphicsWrapper::scoreDevice(VkPhysicalDevice device) {
			VkPhysicalDeviceProperties pProperties;
			vkGetPhysicalDeviceProperties(device, &pProperties);
			std::cout << "\t" << pProperties.deviceName << std::endl;

			QueueFamilyIndices indices = findQueueFamilies(device);

			/*bool extensionsSupported = checkDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported) {
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}*/

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			return (indices.isComplete() /*&& extensionsSupported && swapChainAdequate*/  && supportedFeatures.samplerAnisotropy) * 100;
		}

		void VulkanGraphicsWrapper::getSwapChainRenderTargets(RenderTarget **&rts, uint32_t &rt_count) {
			rts = swap_chain_targets_.data();
			rt_count = swap_chain_targets_.size();
		}

		VulkanGraphicsWrapper::~VulkanGraphicsWrapper() {
			/*
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(device, inFlightFences[i], nullptr);
			}
			*/

			vkDestroyCommandPool(device_, command_pool_graphics_, nullptr);

			vkDestroyDevice(device_, nullptr);

			if (enableValidationLayers) {
				//DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
			}

			vkDestroySurfaceKHR(instance_, surface_, nullptr);
			vkDestroyInstance(instance_, nullptr);

		}

		uint32_t VulkanGraphicsWrapper::GetImageIndex()
		{
			return uint32_t();
		}

		void VulkanGraphicsWrapper::WaitUntilIdle()
		{
		}

		void VulkanGraphicsWrapper::createSyncObjects() {
			imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
			imagesInFlight.resize(swap_chain_targets_.size(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

					throw std::runtime_error("failed to create synchronization objects for a frame!");
				}
			}
		}

		void VulkanGraphicsWrapper::createDescriptorPool() {
			std::array<VkDescriptorPoolSize, 2> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = 1; // static_cast<uint32_t>(swapChainImages.size());
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 1; //static_cast<uint32_t>(swapChainImages.size());

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 20;

			if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptor_pool_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}

		void VulkanGraphicsWrapper::DrawCommandBuffers(uint32_t ii, CommandBuffer ** commandBuffers, uint32_t commandBufferCount) {
			vkWaitForFences(device_, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

			uint32_t imageIndex;
			vkAcquireNextImageKHR(device_, swap_chain_, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

			if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(device_, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
			}
			imagesInFlight[imageIndex] = inFlightFences[currentFrame];

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkCommandBuffer cmd = ((VulkanCommandBuffer *)commandBuffers[imageIndex])->getCommandBuffer();

			VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd;

			VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			vkResetFences(device_, 1, &inFlightFences[currentFrame]);

			if (vkQueueSubmit(graphics_queue_, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { swap_chain_ };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			vkQueuePresentKHR(present_queue_, &presentInfo);

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		}

		ColorFormat VulkanGraphicsWrapper::GetDeviceColorFormat() {
			return swapchain_format_;
		}

		VulkanGraphicsWrapper &VulkanGraphicsWrapper::get() {
			return *graphics_wrapper_;
		}

		uint32_t VulkanGraphicsWrapper::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");
		}

		VkDevice VulkanGraphicsWrapper::getDevice() {
			return device_;
		}

		VkCommandPool VulkanGraphicsWrapper::getGraphicsCommandPool() {
			return command_pool_graphics_;
		}

		void VulkanGraphicsWrapper::CreateDefaultStructures()
		{
		}

		void VulkanGraphicsWrapper::CreateDefaultFramebuffers(DefaultFramebufferCreateInfo ci, Framebuffer **& framebuffers, uint32_t & framebufferCount)
		{
		}

		//==================================
		// Creator
		//==================================
		Framebuffer *VulkanGraphicsWrapper::CreateFramebuffer(FramebufferCreateInfo ci) {
			return static_cast<Framebuffer *>(new VulkanFramebuffer(ci));
		}

		RenderPass * VulkanGraphicsWrapper::CreateRenderPass(RenderPassCreateInfo ci) {
			return static_cast<RenderPass *>(new VulkanRenderPass(ci));
		}

		GraphicsPipeline * VulkanGraphicsWrapper::CreateGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			return static_cast<GraphicsPipeline *>(new VulkanGraphicsPipeline(ci));
		}

		CommandBuffer * VulkanGraphicsWrapper::CreateCommandBuffer(CommandBufferCreateInfo ci) {
			return static_cast<CommandBuffer *>(new VulkanCommandBuffer(ci));
		}

		VertexBuffer * VulkanGraphicsWrapper::CreateVertexBuffer(VertexBufferCreateInfo ci) {
			return static_cast<VertexBuffer *>(new VulkanVertexBuffer(ci));
		}

		IndexBuffer * VulkanGraphicsWrapper::CreateIndexBuffer(IndexBufferCreateInfo ci) {
			return static_cast<IndexBuffer *>(new VulkanIndexBuffer(ci));
		}

		UniformBuffer * VulkanGraphicsWrapper::CreateUniformBuffer(UniformBufferCreateInfo ci) {
			return static_cast<UniformBuffer *>(new VulkanUniformBuffer(ci));
		}

		UniformBufferBinding * VulkanGraphicsWrapper::CreateUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
			return static_cast<UniformBufferBinding *>(new VulkanUniformBufferBinding(ci));
		}

		Texture * VulkanGraphicsWrapper::CreateCubemap(CubemapCreateInfo ci) {
			return nullptr; // static_cast<Texture *>(new VulkanTexture(ci));
		}

		Texture * VulkanGraphicsWrapper::CreateTexture(TextureCreateInfo ci) {
			return static_cast<Texture *>(new VulkanTexture(ci));
		}

		TextureBinding * VulkanGraphicsWrapper::CreateTextureBinding(TextureBindingCreateInfo ci) {
			return static_cast<TextureBinding *>(new VulkanTextureBinding(ci));
		}

		TextureBindingLayout * VulkanGraphicsWrapper::CreateTextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
			return static_cast<TextureBindingLayout *>(new VulkanTextureBindingLayout(ci));
		}

		RenderTarget * VulkanGraphicsWrapper::CreateRenderTarget(RenderTargetCreateInfo * ci, uint32_t rc, bool cube) {
			return static_cast<RenderTarget *>(new VulkanRenderTarget(*ci));
		}

		DepthTarget * VulkanGraphicsWrapper::CreateDepthTarget(DepthTargetCreateInfo ci) {
			return static_cast<DepthTarget *>(new VulkanDepthTarget(ci));
		}

		//==================================
		// Deleter
		//==================================
		void VulkanGraphicsWrapper::DeleteRenderTarget(RenderTarget * ptr) {
			delete (VulkanRenderTarget *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteDepthTarget(DepthTarget * ptr) {
			delete (VulkanDepthTarget *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteFramebuffer(Framebuffer *ptr) {
			delete (VulkanFramebuffer *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteVertexBuffer(VertexBuffer *ptr) {
			delete (VulkanVertexBuffer *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteIndexBuffer(IndexBuffer *ptr) {
			delete (VulkanIndexBuffer *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteUniformBuffer(UniformBuffer *ptr) {
			delete (VulkanUniformBuffer *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteUniformBufferBinding(UniformBufferBinding * ptr) {
			delete (VulkanUniformBufferBinding *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteGraphicsPipeline(GraphicsPipeline *ptr) {
			delete (VulkanGraphicsPipeline *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteRenderPass(RenderPass *ptr) {
			delete (VulkanRenderPass *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteTexture(Texture * ptr) {
			delete (VulkanTexture *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteTextureBinding(TextureBinding * ptr) {
			delete (VulkanTextureBinding *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteTextureBindingLayout(TextureBindingLayout * ptr) {
			delete (VulkanTextureBindingLayout *)ptr;
		}
		void VulkanGraphicsWrapper::DeleteCommandBuffer(CommandBuffer *ptr) {
			delete (VulkanCommandBuffer *)ptr;
		}

		//==================================
		// Booleans
		//==================================
		inline const bool VulkanGraphicsWrapper::shouldUseImmediateMode() {
			return false;
		}
		inline const bool VulkanGraphicsWrapper::supportsCommandBuffers() {
			return false;
		}
		inline const bool VulkanGraphicsWrapper::supportsTesselation() {
			return false;
		}
		inline const bool VulkanGraphicsWrapper::supportsGeometryShader() {
			return false;
		}
		inline const bool VulkanGraphicsWrapper::supportsComputeShader() {
			return false;
		}
		inline const bool VulkanGraphicsWrapper::supportsMultiDrawIndirect() {
			return false;
		}

		//==================================
		// Unused
		//==================================
		VertexArrayObject * VulkanGraphicsWrapper::CreateVertexArrayObject(VertexArrayObjectCreateInfo ci) {
			std::cout << "VulkanGraphicsWrapper::CreateVertexArrayObject is not used.\n";
			assert(false);
			return nullptr;
		}
		void VulkanGraphicsWrapper::DeleteVertexArrayObject(VertexArrayObject * ptr) {
			std::cout << "VulkanGraphicsWrapper::DeleteVertexArrayObject is not used\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::Clear(ClearMode mask) {
			std::cout << "VulkanGraphicsWrapper::Clear is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
			std::cout << "VulkanGraphicsWrapper::setViewport is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::BindTextureBinding(TextureBinding *) {
			std::cout << "VulkanGraphicsWrapper::BindTextureBinding is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::BindVertexArrayObject(VertexArrayObject *) {
			std::cout << "VulkanGraphicsWrapper::BindVertexArrayObject is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::DrawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
			std::cout << "VulkanGraphicsWrapper::DrawImmediateIndexed is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::DrawImmediateVertices(uint32_t base, uint32_t count) {
			std::cout << "VulkanGraphicsWrapper::DrawImmediateVertices is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::SetImmediateBlending(BlendMode) {
			std::cout << "VulkanGraphicsWrapper::SetImmediateBlending is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::EnableDepth(bool state) {
			std::cout << "VulkanGraphicsWrapper::EnableDepth is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::SetColorMask(ColorMask mask) {
			std::cout << "VulkanGraphicsWrapper::SetColorMask is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::SwapBuffer() {
			std::cout << "VulkanGraphicsWrapper::SwapBuffer is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::CopyToDepthBuffer(DepthTarget * p) {
			std::cout << "VulkanGraphicsWrapper::CopyToDepthBuffer is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::BindDefaultFramebuffer(bool depth) {
			std::cout << "VulkanGraphicsWrapper::BindDefaultFramebuffer is not used.\n";
			assert(false);
		}

		//==================================
		// DLL Interface
		//==================================
		GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
			return new VulkanGraphicsWrapper(createInfo);
		}

		void deleteGraphics(void * ptr) {
			VulkanGraphicsWrapper * glptr = (VulkanGraphicsWrapper *)ptr;
			delete glptr;
		}
	}
}