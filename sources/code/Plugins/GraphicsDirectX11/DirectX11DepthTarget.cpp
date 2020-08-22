#include "../pch.h"
#include "DirectX11DepthTarget.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11Format.hpp"
#include "DirectX11Utils.hpp"
#include <assert.h>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11DepthTarget::DirectX11DepthTarget(DepthTargetCreateInfo ci) {
			VkFormat depthFormat = TranslateDepthFormatToDirectX11(ci.format);

			createImage(ci.width, ci.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
			image_view_ = createImageView(image_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		}
		
		DirectX11DepthTarget::~DirectX11DepthTarget() {
			VkDevice device = DirectX11GraphicsWrapper::get().getDevice();
			vkDestroyImageView(device, image_view_, nullptr);
			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		VkImageView DirectX11DepthTarget::getImageView() {
			return image_view_;
		}
		void DirectX11DepthTarget::BindFace(int k) {
			std::cout << "DirectX11DepthTarget::BindFace is not used.\n";
			assert(false);
		}
	}
}
