#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include "VulkanWindowGraphicsBinding.hpp"
#include "VulkanGraphicsWrapper.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCommandBuffer.hpp"
#include "../Window/Win32Window.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		bool VulkanWindowGraphicsBinding::initialize(Window *window) {
			window_ = window;
			max_frames_in_flight_ = 3;

#ifdef VK_USE_PLATFORM_WIN32_KHR
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
			surfaceCreateInfo.hwnd = ((Win32Window *)window)->getHandle();
			surfaceCreateInfo.pNext = VK_NULL_HANDLE;
			surfaceCreateInfo.flags = 0;

			if (vkCreateWin32SurfaceKHR(VulkanGraphicsWrapper::get().getInstance(), &surfaceCreateInfo, nullptr, &surface_) != VK_SUCCESS)
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
			auto gw = VulkanGraphicsWrapper::get();
			
			// Delete rt imageview
			vkDestroySwapchainKHR(gw.getDevice(), swap_chain_, nullptr);
			vkDestroySurfaceKHR(gw.getInstance(), surface_, nullptr);
		}

		VkSurfaceKHR VulkanWindowGraphicsBinding::getSurface() {
			return surface_;
		}

		void VulkanWindowGraphicsBinding::getSwapChainRenderTargets(RenderTarget**& rts, uint32_t& rt_count) {
			rts = swap_chain_targets_.data();
			rt_count = swap_chain_targets_.size();
		}

		ColorFormat VulkanWindowGraphicsBinding::getDeviceColorFormat() {
			return swapchain_format_;
		}

		SwapChainSupportDetails VulkanWindowGraphicsBinding::querySwapChainSupport(VkPhysicalDevice device) {
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

		VkSurfaceFormatKHR VulkanWindowGraphicsBinding::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		VkPresentModeKHR VulkanWindowGraphicsBinding::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D VulkanWindowGraphicsBinding::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != UINT32_MAX) {
				return capabilities.currentExtent;
			}
			else {
				unsigned int width, height;
				window_->getWindowSize(width, height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}



		void VulkanWindowGraphicsBinding::createSyncObjects() {
			auto device = VulkanGraphicsWrapper::get().getDevice();

			image_available_semaphores_.resize(max_frames_in_flight_);
			render_finished_semaphores_.resize(max_frames_in_flight_);
			in_flight_fences_.resize(max_frames_in_flight_);
			images_in_flight_.resize(swap_chain_targets_.size(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < max_frames_in_flight_; i++) {
				if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreInfo, nullptr, &render_finished_semaphores_[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceInfo, nullptr, &in_flight_fences_[i]) != VK_SUCCESS) {

					throw std::runtime_error("Vulkan: Failed to create synchronization objects for a frame!");
				}
			}
		}

		void VulkanWindowGraphicsBinding::presentCommandBuffer(CommandBuffer** buffers, uint32_t num_buffers) {
			auto vkgw = VulkanGraphicsWrapper::get();
			auto device = vkgw.getDevice();
			auto graphics_queue = vkgw.graphics_queue_;
			auto present_queue = vkgw.present_queue_;

			vkWaitForFences(device, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

			uint32_t imageIndex = 0;
			vkAcquireNextImageKHR(device, swap_chain_, UINT64_MAX, image_available_semaphores_[current_frame_], VK_NULL_HANDLE, &imageIndex);

			if (images_in_flight_[imageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(device, 1, &images_in_flight_[imageIndex], VK_TRUE, UINT64_MAX);
			}
			images_in_flight_[imageIndex] = in_flight_fences_[current_frame_];

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkCommandBuffer cmd = ((VulkanCommandBuffer*)buffers[imageIndex])->getCommandBuffer();

			VkSemaphore waitSemaphores[] = { image_available_semaphores_[current_frame_] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd;

			VkSemaphore signalSemaphores[] = { render_finished_semaphores_[current_frame_] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			vkResetFences(device, 1, &in_flight_fences_[current_frame_]);

			if (vkQueueSubmit(graphics_queue, 1, &submitInfo, in_flight_fences_[current_frame_]) != VK_SUCCESS) {
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

			vkQueuePresentKHR(present_queue, &presentInfo);

			current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
		}

		void VulkanWindowGraphicsBinding::createSwapChain() {
			auto physical_device = VulkanGraphicsWrapper::get().getPhysicalDevice();
			auto device = VulkanGraphicsWrapper::get().getDevice();

			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device);

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

			QueueFamilyIndices indices = VulkanGraphicsWrapper::get().findQueueFamilies(physical_device);
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

			if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swap_chain_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}

			std::vector<VkImage> swap_chain_images_;
			vkGetSwapchainImagesKHR(device, swap_chain_, &imageCount, nullptr);
			swap_chain_images_.resize(imageCount);
			swap_chain_targets_.resize(imageCount);
			vkGetSwapchainImagesKHR(device, swap_chain_, &imageCount, swap_chain_images_.data());

			for (uint32_t i = 0; i < imageCount; ++i) {
				swap_chain_targets_[i] = new VulkanRenderTarget(swap_chain_images_[i], surfaceFormat.format);
			}

			swapchain_format_ = TranslateColorFormatFromVulkan(surfaceFormat.format);

			createSyncObjects();
		}
	};
};