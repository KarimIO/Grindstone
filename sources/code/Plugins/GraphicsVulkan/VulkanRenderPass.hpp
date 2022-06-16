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
			VkRenderPass GetRenderPassHandle();
			uint32_t GetWidth();
			uint32_t GetHeight();
		private:
			VkRenderPass renderPass;
			uint32_t width;
			uint32_t height;
		};
	}
}
