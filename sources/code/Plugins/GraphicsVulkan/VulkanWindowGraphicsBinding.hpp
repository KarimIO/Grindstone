#pragma once

#include <Windows.h>
#include <Common/Window/Window.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

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
			virtual bool Initialize(Window *window) override;
			~VulkanWindowGraphicsBinding();
		public:
			VkSurfaceKHR GetSurface();
			void GetSwapChainRenderTargets(RenderTarget**& rts, uint32_t& rt_count);
			ColorFormat GetDeviceColorFormat();
			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void CreateSwapChain();
			void CreateSyncObjects();
			void PresentCommandBuffer(CommandBuffer** buffers, uint32_t num_buffers) override;
		private:
			Window* window;
			
			ColorFormat swapchainFormat;

			VkSurfaceKHR surface;
			VkSwapchainKHR swapChain;
			std::vector<RenderTarget*> swapChainTargets;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			uint32_t currentFrame, maxFramesInFlight;
		};
	};
};
