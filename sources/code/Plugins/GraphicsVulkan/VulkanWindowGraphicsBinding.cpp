#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <Common/Window/GlfwWindow.hpp>
#include <EngineCore/Logger.hpp>

#include "VulkanWindowGraphicsBinding.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandBuffer.hpp"

using namespace Grindstone::GraphicsAPI;

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

VkSurfaceKHR VulkanWindowGraphicsBinding::GetSurface() {
	return surface;
}

VkSwapchainKHR VulkanWindowGraphicsBinding::GetSwapchain() {
	return swapChain;
}

void VulkanWindowGraphicsBinding::SubmitWindowObjects(VulkanWindowBindingDataNative& windowBindingData) {
	swapChain = windowBindingData.swapChain;
	if (renderPass == nullptr) {
		renderPass = new VulkanRenderPass(windowBindingData.renderPass, "Swapchain Render Pass");
	}
	else {
		static_cast<VulkanRenderPass*>(renderPass)->Update(windowBindingData.renderPass);
	}
	swapchainVulkanFormat = windowBindingData.surfaceFormat.format;
	swapchainFormat = TranslateColorFormatFromVulkan(swapchainVulkanFormat);

	imageSets.resize(windowBindingData.imageSetCount);
	for (uint32_t i = 0; i < windowBindingData.imageSetCount; ++i) {
		VulkanImageSetNative& native = windowBindingData.imageSets[i];
		VulkanImageSet& imageSet = imageSets[i];

		if (imageSet.framebuffer == nullptr) {
			imageSet.framebuffer = new VulkanFramebuffer(this->renderPass, native.framebuffer, windowBindingData.width, windowBindingData.height, "Swapchain Framebuffer");
		}
		else {
			static_cast<VulkanFramebuffer*>(imageSet.framebuffer)->UpdateNativeFramebuffer(this->renderPass, native.framebuffer, windowBindingData.width, windowBindingData.height);
		}

		if (imageSet.swapChainTarget == nullptr) {
			imageSet.swapChainTarget = new VulkanRenderTarget(native.image, native.imageView, swapchainVulkanFormat);
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

	VkRenderPass vkRenderPass = static_cast<VulkanRenderPass*>(renderPass)->GetRenderPassHandle();

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = vkRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = swapExtent.width;
	framebufferInfo.height = swapExtent.height;
	framebufferInfo.layers = 1;

	imageSets.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i) {
		VulkanRenderTarget* rt = new VulkanRenderTarget(
			swapChainImages[i],
			swapchainVulkanFormat,
			i
		);

		VkImageView attachments[] = { rt->GetImageView() };
		framebufferInfo.pAttachments = attachments;

		VkFramebuffer vkFramebuffer = nullptr;
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &vkFramebuffer) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create framebuffer!");
		}

		VulkanImageSet& imageSet = imageSets[i];
		imageSet.swapChainTarget = rt;
		imageSet.framebuffer = new VulkanFramebuffer(
			renderPass,
			vkFramebuffer,
			swapExtent.width,
			swapExtent.height,
			"Swapchain Framebuffer"
		);

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
		delete imageSets[i].framebuffer;
		delete imageSets[i].swapChainTarget;
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

	CreateRenderPass();
	CreateImageSets();
}

void VulkanWindowGraphicsBinding::CreateRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainVulkanFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkDevice device = VulkanCore::Get().GetDevice();
	VkRenderPass vkRenderPass = nullptr;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create render pass!");
	}

	if (renderPass == nullptr) {
		renderPass = new VulkanRenderPass(vkRenderPass, "Swapchain Render Pass");
	}
	else {
		static_cast<VulkanRenderPass*>(renderPass)->Update(vkRenderPass);
	}
}

RenderPass* VulkanWindowGraphicsBinding::GetRenderPass() {
	return renderPass;
}

Framebuffer* VulkanWindowGraphicsBinding::GetCurrentFramebuffer() {
	return imageSets[currentFrame].framebuffer;
}

uint32_t VulkanWindowGraphicsBinding::GetCurrentImageIndex() {
	return currentFrame;
}

uint32_t VulkanWindowGraphicsBinding::GetMaxFramesInFlight() {
	return maxFramesInFlight;
}

void VulkanWindowGraphicsBinding::ImmediateSetContext() {}
void VulkanWindowGraphicsBinding::ImmediateSwapBuffers() {}
