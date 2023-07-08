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
			~VulkanWindowGraphicsBinding();

			void CreateSwapChain();
			VkSurfaceKHR GetSurface();
			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

			// Inherited via WindowGraphicsBinding
			virtual bool Initialize(Window *window) override;
			virtual bool AcquireNextImage() override;
			virtual void SubmitCommandBuffer(CommandBuffer* buffers) override;
			virtual bool PresentSwapchain() override;
			virtual RenderPass* GetRenderPass() override;
			virtual Framebuffer* GetCurrentFramebuffer() override;
			virtual uint32_t GetCurrentImageIndex() override;
			virtual uint32_t GetMaxFramesInFlight() override;
			virtual void ImmediateSetContext() override;
			virtual void ImmediateSwapBuffers() override;
			virtual void Resize(uint32_t width, uint32_t height) override;
		private:
			void GetSwapChainRenderTargets(RenderTarget**& renderTargets, uint32_t& renderTargetCount);
			ColorFormat GetDeviceColorFormat();
			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void CreateSyncObjects();
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
			uint32_t currentSwapchainImageIndex = 0;
		};
	};
};
