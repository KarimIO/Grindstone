#pragma once

#include "../GraphicsCommon/RenderTarget.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11RenderTarget : public RenderTarget {
		public:
			//DirectX11RenderTarget(VkImage swapchain_image, VkFormat); // Build from swapchain
			DirectX11RenderTarget(RenderTargetCreateInfo ci);
			virtual ~DirectX11RenderTarget() override;
		public:
			//VkImageView getImageView();
		public:
			virtual float getAverageValue(uint32_t i) override;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) override;
		private:
			//VkImage image_;
			//VkImageView image_view_;
			//VkDeviceMemory image_memory_;
		};
	}
}
