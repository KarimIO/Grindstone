#pragma once

#include "../GraphicsCommon/RenderPass.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11RenderPass : public RenderPass {
		public:
			DirectX11RenderPass(RenderPassCreateInfo rp);
			virtual ~DirectX11RenderPass() {};
		public:
			//VkRenderPass getRenderPassHandle();
			uint32_t getWidth();
			uint32_t getHeight();
		private:
			//VkRenderPass render_pass_;
			uint32_t width_;
			uint32_t height_;
		};
	}
}