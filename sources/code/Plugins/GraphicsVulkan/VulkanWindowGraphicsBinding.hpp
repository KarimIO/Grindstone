#pragma once

#include <Windows.h>
#include "../Window/Window.hpp"
#include "../GraphicsCommon/WindowGraphicsBinding.hpp"

#include "VulkanRenderTarget.hpp"
#include <vector>

namespace Grindstone {
	namespace GraphicsAPI {
		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		class VulkanWindowGraphicsBinding : public WindowGraphicsBinding {
		public:
			virtual bool initialize(Window *window) override;
			~VulkanWindowGraphicsBinding();
		public:
			VkSurfaceKHR getSurface();
			void getSwapChainRenderTargets(RenderTarget**& rts, uint32_t& rt_count);
			ColorFormat getDeviceColorFormat();
			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void createSwapChain();
			void createSyncObjects();
			void presentCommandBuffer(CommandBuffer** buffers, uint32_t num_buffers) override;
		private:
			Window* window_;
			
			ColorFormat swapchain_format_;

			VkSurfaceKHR surface_;
			VkSwapchainKHR swap_chain_;
			std::vector<RenderTarget*> swap_chain_targets_;

			std::vector<VkSemaphore> image_available_semaphores_;
			std::vector<VkSemaphore> render_finished_semaphores_;
			std::vector<VkFence> in_flight_fences_;
			std::vector<VkFence> images_in_flight_;
			uint32_t current_frame_, max_frames_in_flight_;
		};
	};
};