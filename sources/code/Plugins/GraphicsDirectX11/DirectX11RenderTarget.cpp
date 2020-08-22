#include "../pch.h"
#include "DirectX11RenderTarget.hpp"
#include "DirectX11Utils.hpp"
#include "DirectX11Format.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include <iostream>
#include <cassert>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11RenderTarget::DirectX11RenderTarget(VkImage swapchain_image, VkFormat format) : image_(swapchain_image) {
			image_view_ = createImageView(image_, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		DirectX11RenderTarget::DirectX11RenderTarget(RenderTargetCreateInfo ci) {
			uint8_t channels;
			VkFormat render_format = TranslateColorFormatToDirectX11(ci.format, channels);

			createImage(ci.width, ci.height, 1, render_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
			image_view_ = createImageView(image_, render_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		DirectX11RenderTarget::~DirectX11RenderTarget() {
			VkDevice device = DirectX11GraphicsWrapper::get().getDevice();
			vkDestroyImageView(device, image_view_, nullptr);
			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		VkImageView DirectX11RenderTarget::getImageView() {
			return image_view_;
		}

		float DirectX11RenderTarget::getAverageValue(uint32_t i) {
			std::cout << "DirectX11RenderTarget::getAverageValue is not used.\n";
			assert(false);
			return 0.0f;
		}

		void DirectX11RenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
			std::cout << "DirectX11RenderTarget::RenderScreen is not used.\n";
			assert(false);
		}

	}
}