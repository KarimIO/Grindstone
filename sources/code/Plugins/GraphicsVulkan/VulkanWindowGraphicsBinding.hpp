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

		struct VulkanImageSet {
			RenderTarget* swapChainTarget;
			VkFence fence;
		};

		struct VulkanImageSetNative {
			VkImage             image;
			VkImageView         imageView;
			VkFramebuffer       framebuffer;
		};

		struct VulkanWindowBindingDataNative {
			VkSwapchainKHR swapChain;
			uint32_t width;
			uint32_t height;
			uint32_t imageSetCount;
			VkSurfaceFormatKHR surfaceFormat;
			VulkanImageSetNative* imageSets;
		};

		class VulkanWindowGraphicsBinding : public WindowGraphicsBinding {
		public:
			~VulkanWindowGraphicsBinding();

			void CreateSwapChain();
			virtual VkSurfaceKHR GetSurface() const;
			virtual VkSwapchainKHR GetSwapchain() const;
			virtual void SubmitWindowObjects(VulkanWindowBindingDataNative& windowBindingData);
			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
			void CreateSyncObjects();


			// Inherited via WindowGraphicsBinding
			virtual bool Initialize(Window *window) override;
			virtual bool AcquireNextImage() override;
			virtual void SubmitCommandBuffer(CommandBuffer* buffers) override;
			virtual bool PresentSwapchain() override;
			virtual void ImmediateSetContext() override;
			virtual void ImmediateSwapBuffers() override;
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual Grindstone::GraphicsAPI::ColorFormat GetSwapchainFormat() const override;
			virtual uint32_t GetCurrentImageIndex() const override;
			virtual uint32_t GetMaxFramesInFlight() const override;
			virtual Grindstone::GraphicsAPI::RenderTarget* GetSwapchainRenderTarget(uint32_t i) override;
		private:
			ColorFormat GetDeviceColorFormat() const;
			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void CreateImageSets();
			void RecreateSwapchain();
		private:
			bool isSwapchainDirty = false;
			Window* window = nullptr;
			std::vector<VulkanImageSet> imageSets;
			
			ColorFormat swapchainFormat = ColorFormat::Invalid;
			VkFormat swapchainVulkanFormat;

			VkSurfaceKHR surface = nullptr;
			VkSwapchainKHR swapChain = nullptr;

			VkExtent2D swapExtent;
			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			uint32_t currentFrame = 0;
			uint32_t maxFramesInFlight = 0;
			uint32_t currentSwapchainImageIndex = 0;
		};
	};
};
