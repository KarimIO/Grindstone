#define NOMINMAX

#include <cassert>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

#include "VulkanWindowGraphicsBinding.hpp"
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

		bool VulkanGraphicsWrapper::initialize(GraphicsWrapperCreateInfo ci) {
			api_type_ = GraphicsAPIType::Vulkan;
			graphics_wrapper_ = this;
			debug_ = ci.debug;
			primary_window_ = ci.window;

			createInstance();
			auto wgb = new VulkanWindowGraphicsBinding();
			primary_window_->addBinding(wgb);
			wgb->initialize(primary_window_);
			if (ci.debug)
				setupDebugMessenger();
			pickPhysicalDevice();
			createLogicalDevice();
			wgb->createSwapChain();
			createCommandPool();
			createDescriptorPool();

			return true;
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

			if (physical_device_ == VK_NULL_HANDLE) {
				throw std::runtime_error("Vulkan: Failed to find a suitable GPU!");
			}

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physical_device_, &properties);
			const char *vendor_name = getVendorNameFromID(properties.vendorID);
			if (vendor_name == 0) {
				vendor_name_ = std::string("Unknown Vendor(") + std::to_string(properties.vendorID) + ")";
			}
			else {
				vendor_name_ = vendor_name;
			}
			adapter_name_ = properties.deviceName;

			unsigned int ver_maj = (properties.apiVersion >> 22) & 0x3FF;
			unsigned int ver_min = (properties.apiVersion >> 12) & 0x3FF;
			unsigned int ver_patch = (properties.apiVersion) & 0xfff;
			api_version_ = std::to_string(ver_maj)+"."+ std::to_string(ver_min) + "." + std::to_string(ver_patch);

			QueueFamilyIndices indices = findQueueFamilies(physical_device_);
			graphics_family_ = indices.graphicsFamily;
			present_family_ = indices.presentFamily;
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
			auto wgb = ((VulkanWindowGraphicsBinding*)primary_window_->getWindowGraphicsBinding());

			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
					indices.hasGraphicsFamily = true;
				}

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, wgb->getSurface(), &presentSupport);

				if (presentSupport) {
					indices.presentFamily = i;
					indices.hasPresentFamily = true;
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
			//VkPhysicalDeviceProperties pProperties;
			//vkGetPhysicalDeviceProperties(device, &pProperties);

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

		VulkanGraphicsWrapper::~VulkanGraphicsWrapper() {
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroySemaphore(device_, renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(device_, imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(device_, inFlightFences[i], nullptr);
			}

			vkDestroyCommandPool(device_, command_pool_graphics_, nullptr);
			vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);

			vkDestroyDevice(device_, nullptr);

			if (enableValidationLayers) {
				DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
			}

			vkDestroyInstance(instance_, nullptr);

		}
		
		void VulkanGraphicsWrapper::waitUntilIdle() {
			vkDeviceWaitIdle(device_);
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

		VulkanGraphicsWrapper &VulkanGraphicsWrapper::get() {
			return *graphics_wrapper_;
		}

		void VulkanGraphicsWrapper::registerWindow(Window* window) {
			auto wgb = new VulkanWindowGraphicsBinding();
			window->addBinding(wgb);
			wgb->initialize(window);
			//wgb->shareLists((VulkanWindowGraphicsBinding*)primary_window_->getWindowGraphicsBinding());
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

		VkInstance VulkanGraphicsWrapper::getInstance() {
			return instance_;
		}

		VkDevice VulkanGraphicsWrapper::getDevice() {
			return device_;
		}

		VkPhysicalDevice VulkanGraphicsWrapper::getPhysicalDevice() {
			return physical_device_;
		}

		VkCommandPool VulkanGraphicsWrapper::getGraphicsCommandPool() {
			return command_pool_graphics_;
		}


		void VulkanGraphicsWrapper::adjustPerspective(float *perspective) {
			perspective[1*4 + 1] *= -1;
		}

		//==================================
		// Get Text Metainfo
		//==================================
		const char* VulkanGraphicsWrapper::getVendorName() {
			return vendor_name_.c_str();
		}

		const char* VulkanGraphicsWrapper::getAdapterName() {
			return adapter_name_.c_str();
		}

		const char* VulkanGraphicsWrapper::getAPIName() {
			return "Vulkan";
		}

		const char* VulkanGraphicsWrapper::getAPIVersion() {
			return api_version_.c_str();
		}

		//==================================
		// Creators
		//==================================
		Framebuffer *VulkanGraphicsWrapper::createFramebuffer(FramebufferCreateInfo ci) {
			return static_cast<Framebuffer *>(new VulkanFramebuffer(ci));
		}

		RenderPass * VulkanGraphicsWrapper::createRenderPass(RenderPassCreateInfo ci) {
			return static_cast<RenderPass *>(new VulkanRenderPass(ci));
		}

		GraphicsPipeline * VulkanGraphicsWrapper::createGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			return static_cast<GraphicsPipeline *>(new VulkanGraphicsPipeline(ci));
		}

		CommandBuffer * VulkanGraphicsWrapper::createCommandBuffer(CommandBufferCreateInfo ci) {
			return static_cast<CommandBuffer *>(new VulkanCommandBuffer(ci));
		}

		VertexBuffer * VulkanGraphicsWrapper::createVertexBuffer(VertexBufferCreateInfo ci) {
			return static_cast<VertexBuffer *>(new VulkanVertexBuffer(ci));
		}

		IndexBuffer * VulkanGraphicsWrapper::createIndexBuffer(IndexBufferCreateInfo ci) {
			return static_cast<IndexBuffer *>(new VulkanIndexBuffer(ci));
		}

		UniformBuffer * VulkanGraphicsWrapper::createUniformBuffer(UniformBufferCreateInfo ci) {
			return static_cast<UniformBuffer *>(new VulkanUniformBuffer(ci));
		}

		UniformBufferBinding * VulkanGraphicsWrapper::createUniformBufferBinding(UniformBufferBindingCreateInfo ci) {
			return static_cast<UniformBufferBinding *>(new VulkanUniformBufferBinding(ci));
		}

		Texture * VulkanGraphicsWrapper::createCubemap(CubemapCreateInfo ci) {
			return nullptr; // static_cast<Texture *>(new VulkanTexture(ci));
		}

		Texture * VulkanGraphicsWrapper::createTexture(TextureCreateInfo ci) {
			return static_cast<Texture *>(new VulkanTexture(ci));
		}

		TextureBinding * VulkanGraphicsWrapper::createTextureBinding(TextureBindingCreateInfo ci) {
			return static_cast<TextureBinding *>(new VulkanTextureBinding(ci));
		}

		TextureBindingLayout * VulkanGraphicsWrapper::createTextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
			return static_cast<TextureBindingLayout *>(new VulkanTextureBindingLayout(ci));
		}

		RenderTarget * VulkanGraphicsWrapper::createRenderTarget(RenderTargetCreateInfo * ci, uint32_t rc, bool cube) {
			return static_cast<RenderTarget *>(new VulkanRenderTarget(*ci));
		}

		DepthTarget * VulkanGraphicsWrapper::createDepthTarget(DepthTargetCreateInfo ci) {
			return static_cast<DepthTarget *>(new VulkanDepthTarget(ci));
		}

		//==================================
		// Deleters
		//==================================
		void VulkanGraphicsWrapper::deleteRenderTarget(RenderTarget * ptr) {
			delete (VulkanRenderTarget *)ptr;
		}
		void VulkanGraphicsWrapper::deleteDepthTarget(DepthTarget * ptr) {
			delete (VulkanDepthTarget *)ptr;
		}
		void VulkanGraphicsWrapper::deleteFramebuffer(Framebuffer *ptr) {
			delete (VulkanFramebuffer *)ptr;
		}
		void VulkanGraphicsWrapper::deleteVertexBuffer(VertexBuffer *ptr) {
			delete (VulkanVertexBuffer *)ptr;
		}
		void VulkanGraphicsWrapper::deleteIndexBuffer(IndexBuffer *ptr) {
			delete (VulkanIndexBuffer *)ptr;
		}
		void VulkanGraphicsWrapper::deleteUniformBuffer(UniformBuffer *ptr) {
			delete (VulkanUniformBuffer *)ptr;
		}
		void VulkanGraphicsWrapper::deleteUniformBufferBinding(UniformBufferBinding * ptr) {
			delete (VulkanUniformBufferBinding *)ptr;
		}
		void VulkanGraphicsWrapper::deleteGraphicsPipeline(GraphicsPipeline *ptr) {
			delete (VulkanGraphicsPipeline *)ptr;
		}
		void VulkanGraphicsWrapper::deleteRenderPass(RenderPass *ptr) {
			delete (VulkanRenderPass *)ptr;
		}
		void VulkanGraphicsWrapper::deleteTexture(Texture * ptr) {
			delete (VulkanTexture *)ptr;
		}
		void VulkanGraphicsWrapper::deleteTextureBinding(TextureBinding * ptr) {
			delete (VulkanTextureBinding *)ptr;
		}
		void VulkanGraphicsWrapper::deleteTextureBindingLayout(TextureBindingLayout * ptr) {
			delete (VulkanTextureBindingLayout *)ptr;
		}
		void VulkanGraphicsWrapper::deleteCommandBuffer(CommandBuffer *ptr) {
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
		VertexArrayObject * VulkanGraphicsWrapper::createVertexArrayObject(VertexArrayObjectCreateInfo ci) {
			std::cout << "VulkanGraphicsWrapper::createVertexArrayObject is not used.\n";
			assert(false);
			return nullptr;
		}
		void VulkanGraphicsWrapper::deleteVertexArrayObject(VertexArrayObject * ptr) {
			std::cout << "VulkanGraphicsWrapper::deleteVertexArrayObject is not used\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::clear(ClearMode mask, float clear_color[4], float clear_depth, uint32_t clear_stencil) {
			std::cout << "VulkanGraphicsWrapper::clear is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::bindTextureBinding(TextureBinding *) {
			std::cout << "VulkanGraphicsWrapper::bindTextureBinding is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::bindVertexArrayObject(VertexArrayObject *) {
			std::cout << "VulkanGraphicsWrapper::bindVertexArrayObject is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::drawImmediateIndexed(GeometryType geom_type, bool largeBuffer, int32_t baseVertex, uint32_t indexOffsetPtr, uint32_t indexCount) {
			std::cout << "VulkanGraphicsWrapper::drawImmediateIndexed is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::drawImmediateVertices(GeometryType geom_type, uint32_t base, uint32_t count) {
			std::cout << "VulkanGraphicsWrapper::drawImmediateVertices is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::setImmediateBlending(BlendMode) {
			std::cout << "VulkanGraphicsWrapper::SetImmediateBlending is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::enableDepth(bool state) {
			std::cout << "VulkanGraphicsWrapper::enableDepth is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::setColorMask(ColorMask mask) {
			std::cout << "VulkanGraphicsWrapper::setColorMask is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::copyToDepthBuffer(DepthTarget * p) {
			std::cout << "VulkanGraphicsWrapper::copyToDepthBuffer is not used.\n";
			assert(false);
		}
		void VulkanGraphicsWrapper::bindDefaultFramebuffer(bool depth) {
			std::cout << "VulkanGraphicsWrapper::bindDefaultFramebuffer is not used.\n";
			assert(false);
		}

		//==================================
		// DLL Interface
		//==================================
		/*GraphicsWrapper* createGraphics(InstanceCreateInfo createInfo) {
			return new VulkanGraphicsWrapper(createInfo);
		}

		void deleteGraphics(void * ptr) {
			VulkanGraphicsWrapper * glptr = (VulkanGraphicsWrapper *)ptr;
			delete glptr;
		}*/
	}
}