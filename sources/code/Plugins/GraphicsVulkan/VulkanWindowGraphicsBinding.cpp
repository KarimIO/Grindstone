#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include "VulkanWindowGraphicsBinding.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCommandBuffer.hpp"
#include <Common/Window/Win32Window.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		bool VulkanWindowGraphicsBinding::Initialize(Window *window) {
			window = window;
			maxFramesInFlight = 3;

#ifdef VK_USE_PLATFORM_WIN32_KHR
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.hwnd = ((Win32Window *)window)->GetHandle();
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateWin32SurfaceKHR(VulkanCore::Get().GetInstance(), &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
				return false;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.dpy = xDisplay;
			surfaceCreateInfo.window = xWindow;
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateXlibSurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS)
				return false;
#endif

			return true;
		}

		VulkanWindowGraphicsBinding::~VulkanWindowGraphicsBinding() {
			auto gw = VulkanCore::Get();
			
			// Delete rt imageview
			vkDestroySwapchainKHR(gw.GetDevice(), swapChain, nullptr);
			vkDestroySurfaceKHR(gw.GetInstance(), surface, nullptr);
		}

		VkSurfaceKHR VulkanWindowGraphicsBinding::GetSurface() {
			return surface;
		}

		void VulkanWindowGraphicsBinding::GetSwapChainRenderTargets(RenderTarget**& rts, uint32_t& rt_count) {
			rts = swapChainTargets.data();
			rt_count = swapChainTargets.size();
		}

		ColorFormat VulkanWindowGraphicsBinding::GetDeviceColorFormat() {
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

			imageAvailableSemaphores.resize(maxFramesInFlight);
			renderFinishedSemaphores.resize(maxFramesInFlight);
			inFlightFences.resize(maxFramesInFlight);
			imagesInFlight.resize(swapChainTargets.size(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < maxFramesInFlight; i++) {
				if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

					throw std::runtime_error("Vulkan: Failed to create synchronization objects for a frame!");
				}
			}
		}

		void VulkanWindowGraphicsBinding::PresentCommandBuffer(CommandBuffer** buffers, uint32_t num_buffers) {
			auto vkgw = VulkanCore::Get();
			auto device = vkgw.GetDevice();
			auto graphics_queue = vkgw.graphicsQueue;
			auto present_queue = vkgw.presentQueue;

			vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

			uint32_t imageIndex = 0;
			vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

			if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
			}

			imagesInFlight[imageIndex] = inFlightFences[currentFrame];

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkCommandBuffer cmd = ((VulkanCommandBuffer*)buffers[imageIndex])->getCommandBuffer();

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

			vkResetFences(device, 1, &inFlightFences[currentFrame]);

			if (vkQueueSubmit(graphics_queue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
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

			vkQueuePresentKHR(present_queue, &presentInfo);

			currentFrame = (currentFrame + 1) % maxFramesInFlight;
		}

		void VulkanWindowGraphicsBinding::CreateSwapChain() {
			auto physical_device = VulkanCore::Get().GetPhysicalDevice();
			auto device = VulkanCore::Get().GetDevice();

			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physical_device);

			VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = VulkanCore::Get().FindQueueFamilies(physical_device);
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

			std::vector<VkImage> swapChainImages;
			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			swapChainTargets.resize(imageCount);
			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

			for (uint32_t i = 0; i < imageCount; ++i) {
				swapChainTargets[i] = new VulkanRenderTarget(swapChainImages[i], surfaceFormat.format);
			}

			swapchainFormat = TranslateColorFormatFromVulkan(surfaceFormat.format);

			CreateSyncObjects();
		}
	};
};
