#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <Common/Window/GlfwWindow.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/Logger.hpp>

#include "VulkanWindowGraphicsBinding.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandBuffer.hpp"

using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Memory;

bool VulkanWindowGraphicsBinding::Initialize(Window *window) {
	this->window = window;
	maxFramesInFlight = 3;

	VkResult err = glfwCreateWindowSurface(VulkanCore::Get().GetInstance(), static_cast<GlfwWindow*>(window)->GetHandle(), NULL, &surface);
	if (err) {
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

	if (vkCreateWin32SurfaceKHR(VulkanCore::Get().GetInstance(), &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS) {
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

VulkanWindowGraphicsBinding::~VulkanWindowGraphicsBinding() {
	VulkanCore& vkCore = VulkanCore::Get();
			
	// Delete rt imageview
	vkDestroySwapchainKHR(vkCore.GetDevice(), swapChain, nullptr);
	vkDestroySurfaceKHR(vkCore.GetInstance(), surface, nullptr);
}

VkSurfaceKHR VulkanWindowGraphicsBinding::GetSurface() const {
	return surface;
}

VkSwapchainKHR VulkanWindowGraphicsBinding::GetSwapchain() const {
	return swapChain;
}

void VulkanWindowGraphicsBinding::SubmitWindowObjects(VulkanWindowBindingDataNative& windowBindingData) {
	swapChain = windowBindingData.swapChain;
	swapchainVulkanFormat = windowBindingData.surfaceFormat.format;
	swapchainFormat = TranslateColorFormatFromVulkan(swapchainVulkanFormat);

	imageSets.resize(windowBindingData.imageSetCount);
	for (uint32_t i = 0; i < windowBindingData.imageSetCount; ++i) {
		VulkanImageSetNative& native = windowBindingData.imageSets[i];
		VulkanImageSet& imageSet = imageSets[i];

		if (imageSet.swapChainTarget == nullptr) {
			imageSet.swapChainTarget = AllocatorCore::Allocate<VulkanRenderTarget>(native.image, native.imageView, swapchainVulkanFormat);
		}
		else {
			static_cast<VulkanRenderTarget*>(imageSet.swapChainTarget)->UpdateNativeImage(native.image, native.imageView, swapchainVulkanFormat);
		}
	}
}
ColorFormat VulkanWindowGraphicsBinding::GetDeviceColorFormat() const {
	return swapchainFormat;
}

SwapChainSupportDetails VulkanWindowGraphicsBinding::QuerySwapChainSupport(VkPhysicalDevice device) {
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

VkSurfaceFormatKHR VulkanWindowGraphicsBinding::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const VkSurfaceFormatKHR& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanWindowGraphicsBinding::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const VkPresentModeKHR& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanWindowGraphicsBinding::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

		actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanWindowGraphicsBinding::CreateSyncObjects() {
	VkDevice device = VulkanCore::Get().GetDevice();

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

			GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan: Failed to create synchronization objects for a frame!");
		}
	}
}

void VulkanWindowGraphicsBinding::CreateImageSets() {
	VkDevice device = VulkanCore::Get().GetDevice();

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	std::vector<VkImage> swapChainImages;
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	imageSets.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i) {
		VulkanRenderTarget* rt = AllocatorCore::Allocate<VulkanRenderTarget>(
			swapChainImages[i],
			swapchainVulkanFormat,
			i
		);

		VkImageView attachments[] = { rt->GetImageView() };

		VulkanImageSet& imageSet = imageSets[i];
		imageSet.swapChainTarget = rt;

		imageSet.fence = nullptr;
	}
}

bool VulkanWindowGraphicsBinding::AcquireNextImage() {
	VulkanCore& vkCore = VulkanCore::Get();
	VkDevice device = vkCore.GetDevice();

	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentSwapchainImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to acquire swap chain image!");
	}

	VulkanImageSet& imageSet = imageSets[currentFrame];
	if (imageSet.fence != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imageSet.fence, VK_TRUE, UINT64_MAX);
	}

	vkResetFences(device, 1, &inFlightFences[currentFrame]);
	return true;
}

void VulkanWindowGraphicsBinding::Resize(uint32_t width, uint32_t height) {
	isSwapchainDirty = true;
}

Grindstone::GraphicsAPI::ColorFormat VulkanWindowGraphicsBinding::GetSwapchainFormat() const {
	return swapchainFormat;
}

void VulkanWindowGraphicsBinding::RecreateSwapchain() {
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

	VulkanCore& vkCore = VulkanCore::Get();
	vkCore.WaitUntilIdle();

	if (swapChain != nullptr) {
		VkDevice device = vkCore.GetDevice();
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = nullptr;
	}

	for (size_t i = 0; i < imageSets.size(); i++) {
		AllocatorCore::Free(imageSets[i].swapChainTarget);
	}
	imageSets.clear();

	CreateSwapChain();

	grindstoneGlfwWindow->OnSwapchainResized(width, height);
}

void VulkanWindowGraphicsBinding::SubmitCommandBuffer(CommandBuffer* buffer) {
	VulkanCore& vkCore = VulkanCore::Get();
	VkDevice device = vkCore.GetDevice();
	VkQueue graphicsQueue = vkCore.graphicsQueue;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkCommandBuffer vkCommandBuffer = static_cast<VulkanCommandBuffer*>(buffer)->GetCommandBuffer();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		GPRINT_ERROR(LogSource::GraphicsAPI, "Failed to submit draw command buffer!");
	}
}

bool VulkanWindowGraphicsBinding::PresentSwapchain() {
	VulkanCore& vkCore = VulkanCore::Get();
	VkQueue presentQueue = vkCore.presentQueue;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &currentSwapchainImageIndex;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || isSwapchainDirty) {
		RecreateSwapchain();
		isSwapchainDirty = false;
		return false;
	}

	currentFrame = (currentFrame + 1) % maxFramesInFlight;

	return true;
}

void VulkanWindowGraphicsBinding::CreateSwapChain() {
	VkPhysicalDevice physicalDevice = VulkanCore::Get().GetPhysicalDevice();
	VkDevice device = VulkanCore::Get().GetDevice();

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	swapExtent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	swapchainVulkanFormat = surfaceFormat.format;
	swapchainFormat = TranslateColorFormatFromVulkan(swapchainVulkanFormat);

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = swapchainVulkanFormat;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = VulkanCore::Get().FindQueueFamilies(physicalDevice);
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

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create swap chain!");
	}

	CreateImageSets();
}

uint32_t VulkanWindowGraphicsBinding::GetCurrentImageIndex() const {
	return currentFrame;
}

uint32_t VulkanWindowGraphicsBinding::GetMaxFramesInFlight() const  {
	return maxFramesInFlight;
}

Grindstone::GraphicsAPI::RenderTarget* VulkanWindowGraphicsBinding::GetSwapchainRenderTarget(uint32_t i) {
	return imageSets[i].swapChainTarget;
}

void VulkanWindowGraphicsBinding::ImmediateSetContext() {}
void VulkanWindowGraphicsBinding::ImmediateSwapBuffers() {}
