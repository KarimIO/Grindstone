#pragma once

#include <Common/Graphics/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(RenderPass::CreateInfo &rp);
			virtual ~VulkanRenderPass() override;
		public:
			VkRenderPass getRenderPassHandle();
			uint32_t getWidth();
			uint32_t getHeight();
		private:
			VkRenderPass render_pass_;
			uint32_t width_;
			uint32_t height_;
		};
	}
}