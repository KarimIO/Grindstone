#pragma once

#include <Windows.h>
#include <Common/Window/Window.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

#include "VulkanCore.hpp"
#include "VulkanRenderTarget.hpp"
#include <vector>

namespace Grindstone {
	namespace GraphicsAPI {
		class Framebuffer;
		class RenderPass;

		class VulkanWindowGraphicsBinding : public WindowGraphicsBinding {
		public:
			virtual bool Initialize(Window *window) override;
			~VulkanWindowGraphicsBinding();
		public:
			VkSurfaceKHR GetSurface();
			void GetSwapChainRenderTargets(RenderTarget**& renderTargets, uint32_t& renderTargetCount);
			ColorFormat GetDeviceColorFormat();
			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void CreateSwapChain();
			void CreateSyncObjects();
			void PresentCommandBuffer(CommandBuffer** buffers, uint32_t bufferCount) override;
			virtual RenderPass* GetRenderPass() override;
			virtual Framebuffer* GetCurrentFramebuffer() override;
		private:
			void CreateRenderPass();
			void CreateFramebuffers();
		private:
			Window* window = nullptr;
			RenderPass* renderPass = nullptr;
			std::vector<Framebuffer*> framebuffers;
			
			ColorFormat swapchainFormat = ColorFormat::Invalid;
			VkFormat swapchainVulkanFormat;

			VkSurfaceKHR surface = nullptr;
			VkSwapchainKHR swapChain = nullptr;
			std::vector<RenderTarget*> swapChainTargets;

			VkExtent2D swapExtent;
			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			uint32_t currentFrame = 0;
			uint32_t maxFramesInFlight = 0;
		};
	};
};
