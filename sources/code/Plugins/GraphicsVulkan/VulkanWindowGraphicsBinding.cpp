#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <glfw/glfw3.h>

#include "Common/Window/GlfwWindow.hpp"

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
	renderPass = new VulkanRenderPass(windowBindingData.renderPass, windowBindingData.width, windowBindingData.height);
	swapchainVulkanFormat = windowBindingData.surfaceFormat.format;
	swapchainFormat = TranslateColorFormatFromVulkan(swapchainVulkanFormat);


	imageSets.resize(windowBindingData.imageSetCount);
	for (uint32_t i = 0; i < windowBindingData.imageSetCount; ++i) {
		auto& native = windowBindingData.imageSets[i];
		auto& imageSet = imageSets[i];

		imageSet.framebuffer = new VulkanFramebuffer(this->renderPass, native.framebuffer);
		imageSet.swapChainTarget = new VulkanRenderTarget(native.image, native.imageView, swapchainVulkanFormat);
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
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanWindowGraphicsBinding::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
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
	auto device = VulkanCore::Get().GetDevice();

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

			throw std::runtime_error("Vulkan: Failed to create synchronization objects for a frame!");
		}
	}
}

void VulkanWindowGraphicsBinding::CreateImageSets() {
	auto device = VulkanCore::Get().GetDevice();

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
		VulkanRenderTarget* rt = new VulkanRenderTarget(swapChainImages[i], swapchainVulkanFormat);

		VkImageView attachments[] = { rt->GetImageView() };

		VkFramebuffer vkFramebuffer = nullptr;
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &vkFramebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

		auto& imageSet = imageSets[i];
		imageSet.swapChainTarget = rt;
		imageSet.framebuffer = new VulkanFramebuffer(renderPass, vkFramebuffer);
		imageSet.fence = nullptr;
	}
}

bool VulkanWindowGraphicsBinding::AcquireNextImage() {
	auto& vkCore = VulkanCore::Get();
	auto device = vkCore.GetDevice();

	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentSwapchainImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		return false;
	}

	auto& imageSet = imageSets[currentSwapchainImageIndex];
	if (imageSet.fence != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imageSet.fence, VK_TRUE, UINT64_MAX);
	}

	vkResetFences(device, 1, &inFlightFences[currentFrame]);
	return true;
}

void VulkanWindowGraphicsBinding::Resize(uint32_t width, uint32_t height) {
	return;
	auto& vkCore = VulkanCore::Get();
	VkDevice device = vkCore.GetDevice();

	if (swapChain != nullptr) {
		vkCore.WaitUntilIdle();
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = nullptr;

		for (size_t i = 0; i < imageSets.size(); i++) {
			delete imageSets[i].framebuffer;
			delete imageSets[i].swapChainTarget;
		}

		imageSets.clear();
	}
}

void VulkanWindowGraphicsBinding::SubmitCommandBuffer(CommandBuffer* buffer) {
	auto& vkCore = VulkanCore::Get();
	auto device = vkCore.GetDevice();
	auto graphicsQueue = vkCore.graphicsQueue;
	auto presentQueue = vkCore.presentQueue;

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
		vkCore.logFunction(LogSeverity::Error, "Failed to submit draw command buffer!");
	}
}

bool VulkanWindowGraphicsBinding::PresentSwapchain() {
	auto& vkCore = VulkanCore::Get();
	auto presentQueue = vkCore.presentQueue;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &currentSwapchainImageIndex;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		return false;
	}

	currentFrame = (currentFrame + 1) % maxFramesInFlight;

	return true;
}

void VulkanWindowGraphicsBinding::CreateSwapChain() {
	auto physicalDevice = VulkanCore::Get().GetPhysicalDevice();
	auto device = VulkanCore::Get().GetDevice();

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	swapExtent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
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
		throw std::runtime_error("failed to create swap chain!");
	}

	CreateRenderPass();
	CreateImageSets();
	CreateSyncObjects();
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

	auto device = VulkanCore::Get().GetDevice();
	VkRenderPass vkRenderPass = nullptr;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	if (renderPass == nullptr) {
		renderPass = new VulkanRenderPass(vkRenderPass, swapExtent.width, swapExtent.height);
	}
	else {
		static_cast<VulkanRenderPass*>(renderPass)->Update(vkRenderPass, swapExtent.width, swapExtent.height);
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
