#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <algorithm>

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>

#include <Common/Window/GlfwWindow.hpp>
#include <EngineCore/Logger.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanWindowGraphicsBinding.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFramebuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCommandBuffer.hpp>

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

static const char* VkResultToString(VkResult result) {
	switch (result) {
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_EVENT_SET: return "VK_EVENT_SET";
	case VK_EVENT_RESET: return "VK_EVENT_RESET";
	case VK_INCOMPLETE: return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
	case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
	case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
	case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
	case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
	case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
	case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
	}

	return "Unknown VK Error Result";
}

PFN_vkGetDeviceFaultInfoEXT GetDeviceFaultInfoEXT = VK_NULL_HANDLE;

static void WaitForAftermathCrash() {
	GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)vkGetInstanceProcAddr(Vulkan::Core::Get().GetInstance(), "vkGetDeviceFaultInfoEXT");

	VkDevice device = Vulkan::Core::Get().GetDevice();
	// Query number of available results
	VkDeviceFaultCountsEXT faultCounts{
		.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT
	};

	GetDeviceFaultInfoEXT(device, &faultCounts, NULL);

	std::vector<VkDeviceFaultAddressInfoEXT> addressInfos;
	addressInfos.resize(faultCounts.addressInfoCount);
	std::vector<VkDeviceFaultVendorInfoEXT> vendorInfos;
	vendorInfos.resize(faultCounts.vendorInfoCount);
	std::vector<char> vendorBinaryData;
	vendorBinaryData.resize(faultCounts.vendorBinarySize);

	// Allocate output arrays and query fault data
	VkDeviceFaultInfoEXT faultInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT,
		.pAddressInfos = addressInfos.data(),
		.pVendorInfos = vendorInfos.data(),
		.pVendorBinaryData = vendorBinaryData.data()
	};

	GetDeviceFaultInfoEXT(device, &faultCounts, &faultInfo);

	GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
	GFSDK_Aftermath_GetCrashDumpStatus(&status);

	auto tdrTerminationTimeout = std::chrono::seconds(3);
	auto tStart = std::chrono::steady_clock::now();
	auto tElapsed = std::chrono::milliseconds::zero();

	while (
		status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
		status != GFSDK_Aftermath_CrashDump_Status_Finished &&
		tElapsed < tdrTerminationTimeout
	) {
		// Sleep 50ms and poll the status again until timeout or Aftermath finished processing the crash dump.
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		GFSDK_Aftermath_GetCrashDumpStatus(&status);

		auto tEnd = std::chrono::steady_clock::now();
		tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
	}

	if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
	{
		std::stringstream err_msg;
		err_msg << "Unexpected crash dump status: " << status;
	}
}

bool Vulkan::WindowGraphicsBinding::Initialize(Window *window) {
	this->window = window;
	maxFramesInFlight = 3;

	VkResult result = glfwCreateWindowSurface(Vulkan::Core::Get().GetInstance(), static_cast<GlfwWindow*>(window)->GetHandle(), NULL, &surface);
	if (result != VK_SUCCESS) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to create window surface ({})!", VkResultToString(result));
		return false;
	}

	/*
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = static_cast<Win32Window *>(window)->GetHandle();
	surfaceCreateInfo.pNext = VK_NULL_HANDLE;
	surfaceCreateInfo.flags = 0;

	if (vkCreateWin32SurfaceKHR(Vulkan::Core::Get().GetInstance(), &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS) {
		return false;
	}

#elif defined(VK_USE_PLATFORM_XLIB_KHR)

	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
	surfaceCreateInfo.dpy = xDisplay;
	surfaceCreateInfo.window = xWindow;
	surfaceCreateInfo.pNext = VK_NULL_HANDLE;
	surfaceCreateInfo.flags = 0;

	if (vkCreateXlibSurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS) {
		return false;
	}
#endif
	*/

	return true;
}

Vulkan::WindowGraphicsBinding::~WindowGraphicsBinding() {
	Vulkan::Core& vkCore = Vulkan::Core::Get();
			
	// Delete rt imageview
	vkDestroySwapchainKHR(vkCore.GetDevice(), swapChain, nullptr);
	vkDestroySurfaceKHR(vkCore.GetInstance(), surface, nullptr);
}

VkSurfaceKHR Vulkan::WindowGraphicsBinding::GetSurface() {
	return surface;
}

VkSwapchainKHR Vulkan::WindowGraphicsBinding::GetSwapchain() {
	return swapChain;
}

void Vulkan::WindowGraphicsBinding::SubmitWindowObjects(WindowBindingDataNative& windowBindingData) {
	swapChain = windowBindingData.swapChain;
	if (renderPass == nullptr) {
		renderPass = new Vulkan::RenderPass(windowBindingData.renderPass, "Swapchain Render Pass");
	}
	else {
		static_cast<RenderPass*>(renderPass)->Update(windowBindingData.renderPass);
	}
	swapchainVulkanFormat = windowBindingData.surfaceFormat.format;
	swapchainFormat = TranslateFormatFromVulkan(swapchainVulkanFormat);

	imageSets.resize(windowBindingData.imageSetCount);
	for (uint32_t i = 0; i < windowBindingData.imageSetCount; ++i) {
		ImageSetNative& native = windowBindingData.imageSets[i];
		ImageSet& imageSet = imageSets[i];

		if (imageSet.framebuffer == nullptr) {
			imageSet.framebuffer = new Vulkan::Framebuffer(this->renderPass, native.framebuffer, windowBindingData.width, windowBindingData.height, "Swapchain Framebuffer");
		}
		else {
			static_cast<Framebuffer*>(imageSet.framebuffer)->UpdateNativeFramebuffer(this->renderPass, native.framebuffer, windowBindingData.width, windowBindingData.height);
		}

		if (imageSet.swapChainTarget == nullptr) {
			imageSet.swapChainTarget = new Vulkan::Image(native.image, swapchainVulkanFormat, i);
		}
		else {
			static_cast<Vulkan::Image*>(imageSet.swapChainTarget)->UpdateNativeImage(native.image, native.imageView, swapchainVulkanFormat);
		}
	}
}

Vulkan::SwapChainSupportDetails Vulkan::WindowGraphicsBinding::QuerySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Vulkan::WindowGraphicsBinding::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const VkSurfaceFormatKHR& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Vulkan::WindowGraphicsBinding::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const VkPresentModeKHR& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::WindowGraphicsBinding::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		unsigned int width, height;
		window->GetWindowSize(width, height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void Vulkan::WindowGraphicsBinding::CreateSyncObjects() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	imageAvailableSemaphores.resize(maxFramesInFlight);
	renderFinishedSemaphores.resize(maxFramesInFlight);
	inFlightFences.resize(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			WaitForAftermathCrash();
			GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan: Failed to create synchronization objects for a frame!");
		}
	}
}

void Vulkan::WindowGraphicsBinding::CreateImageSets() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	std::vector<VkImage> swapChainImages;
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	VkRenderPass vkRenderPass = static_cast<RenderPass*>(renderPass)->GetRenderPassHandle();

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = vkRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = swapExtent.width;
	framebufferInfo.height = swapExtent.height;
	framebufferInfo.layers = 1;

	imageSets.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i) {

		std::string imageDebugName = std::string("Swapchain Image ") + std::to_string(i);
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, swapChainImages[i], imageDebugName.c_str());

		Vulkan::Image* rt = Memory::AllocatorCore::Allocate<Vulkan::Image>(
			swapChainImages[i],
			swapchainVulkanFormat,
			i
		);

		VkImageView attachments[] = { rt->GetImageView() };
		framebufferInfo.pAttachments = attachments;

		VkFramebuffer vkFramebuffer = nullptr;
		VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &vkFramebuffer);
		if (result != VK_SUCCESS) {
			WaitForAftermathCrash();
			GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to create framebuffer ({})", VkResultToString(result));
		}

		Vulkan::ImageSet& imageSet = imageSets[i];
		imageSet.swapChainTarget = rt;
		imageSet.framebuffer = Memory::AllocatorCore::Allocate<Vulkan::Framebuffer>(
			renderPass,
			vkFramebuffer,
			swapExtent.width,
			swapExtent.height,
			"Swapchain Framebuffer"
		);

		imageSet.fence = nullptr;
	}
}

void Vulkan::WindowGraphicsBinding::WaitForRenderingFence() {
	Vulkan::Core& vkCore = Vulkan::Core::Get();
	VkDevice device = vkCore.GetDevice();
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

bool Vulkan::WindowGraphicsBinding::AcquireNextImage() {
	Vulkan::Core& vkCore = Vulkan::Core::Get();
	VkDevice device = vkCore.GetDevice();

	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentSwapchainImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to acquire swap chain image! ({})!", VkResultToString(result));
	}

	Vulkan::ImageSet& imageSet = imageSets[currentFrame];
	if (imageSet.fence != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imageSet.fence, VK_TRUE, UINT64_MAX);
	}

	vkResetFences(device, 1, &inFlightFences[currentFrame]);
	return true;
}

void Vulkan::WindowGraphicsBinding::Resize(uint32_t width, uint32_t height) {
	isSwapchainDirty = true;
}

void Vulkan::WindowGraphicsBinding::RecreateSwapchain() {
	if (!window->IsSwapchainControlledByEngine()) {
		return;
	}

	GlfwWindow* grindstoneGlfwWindow = static_cast<GlfwWindow*>(window);
	GLFWwindow* win = grindstoneGlfwWindow->GetHandle();

	int width = 0, height = 0;
	glfwGetFramebufferSize(win, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(win, & width, & height);
		glfwWaitEvents();
	}

	Vulkan::Core& vkCore = Vulkan::Core::Get();
	vkCore.WaitUntilIdle();

	if (swapChain != nullptr) {
		VkDevice device = vkCore.GetDevice();
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = nullptr;

	}

	for (size_t i = 0; i < imageSets.size(); i++) {
		delete imageSets[i].framebuffer;
		delete imageSets[i].swapChainTarget;
	}
	imageSets.clear();

	CreateSwapChain();

	grindstoneGlfwWindow->OnSwapchainResized(width, height);
}

void Vulkan::WindowGraphicsBinding::SubmitCommandBufferNoSynchronization(GraphicsAPI::CommandBuffer* buffer) {
	Vulkan::Core& vkCore = Vulkan::Core::Get();
	VkDevice device = vkCore.GetDevice();
	VkQueue graphicsQueue = vkCore.graphicsQueue;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkCommandBuffer vkCommandBuffer = static_cast<Vulkan::CommandBuffer*>(buffer)->GetCommandBuffer();

	VkFence fence;
	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(device, &fenceInfo, nullptr, &fence);

	submitInfo.waitSemaphoreCount = 0u;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;
	submitInfo.signalSemaphoreCount = 0u;
	submitInfo.pSignalSemaphores = nullptr;

	VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
	if (result != VK_SUCCESS) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to submit draw command buffer ({})!", VkResultToString(result));
	}

	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(device, fence, nullptr);
}

void Vulkan::WindowGraphicsBinding::SubmitCommandBufferForCurrentFrame(GraphicsAPI::CommandBuffer* buffer) {
	Vulkan::Core& vkCore = Vulkan::Core::Get();
	VkDevice device = vkCore.GetDevice();
	VkQueue graphicsQueue = vkCore.graphicsQueue;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkCommandBuffer vkCommandBuffer = static_cast<Vulkan::CommandBuffer*>(buffer)->GetCommandBuffer();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
	if (result != VK_SUCCESS) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to submit draw command buffer ({})!", VkResultToString(result));
	}
}

bool Vulkan::WindowGraphicsBinding::PresentSwapchain() {
	Vulkan::Core& vkCore = Vulkan::Core::Get();
	VkQueue presentQueue = vkCore.presentQueue;

	VkSwapchainKHR swapChains[] { swapChain };
	VkPresentInfoKHR presentInfo {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
		.swapchainCount = 1,
		.pSwapchains = swapChains,
		.pImageIndices = &currentSwapchainImageIndex
	};

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || isSwapchainDirty) {
		RecreateSwapchain();
		isSwapchainDirty = false;
		return false;
	}
	else if (result != VK_SUCCESS) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to present queue ({})!", VkResultToString(result));
	}

	currentFrame = (currentFrame + 1) % maxFramesInFlight;

	return true;
}

void Vulkan::WindowGraphicsBinding::CreateSwapChain() {
	VkPhysicalDevice physicalDevice = Vulkan::Core::Get().GetPhysicalDevice();
	VkDevice device = Vulkan::Core::Get().GetDevice();

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	swapExtent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	swapchainVulkanFormat = surfaceFormat.format;
	swapchainFormat = TranslateFormatFromVulkan(swapchainVulkanFormat);

	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = swapchainVulkanFormat,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = swapExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	};

	QueueFamilyIndices indices = Vulkan::Core::Get().FindQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

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

	VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to create swap chain ({})!", VkResultToString(result));
	}

	CreateRenderPass();
	CreateImageSets();
}

void Vulkan::WindowGraphicsBinding::CreateRenderPass() {
	VkAttachmentDescription colorAttachment{
		.format = swapchainVulkanFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef
	};

	VkSubpassDependency dependency{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	};

	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	VkDevice device = Vulkan::Core::Get().GetDevice();
	VkRenderPass vkRenderPass = nullptr;
	VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &vkRenderPass);
	if (result != VK_SUCCESS) {
		WaitForAftermathCrash();
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to create render pass ({})", VkResultToString(result));
	}

	if (renderPass == nullptr) {
		renderPass = new Vulkan::RenderPass(vkRenderPass, "Swapchain Render Pass");
	}
	else {
		static_cast<Vulkan::RenderPass*>(renderPass)->Update(vkRenderPass);
	}
}

Base::RenderPass* Vulkan::WindowGraphicsBinding::GetRenderPass() {
	return renderPass;
}

Base::Framebuffer* Vulkan::WindowGraphicsBinding::GetCurrentFramebuffer() {
	return imageSets[currentFrame].framebuffer;
}

uint32_t Vulkan::WindowGraphicsBinding::GetCurrentImageIndex() {
	return currentFrame;
}

uint32_t Vulkan::WindowGraphicsBinding::GetMaxFramesInFlight() {
	return maxFramesInFlight;
}

void Vulkan::WindowGraphicsBinding::ImmediateSetContext() {}
void Vulkan::WindowGraphicsBinding::ImmediateSwapBuffers() {}
