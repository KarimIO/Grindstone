#pragma once

#include <Common/Graphics/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(VkRenderPass renderPass, uint32_t width, uint32_t height);
			VulkanRenderPass(RenderPass::CreateInfo& createInfo);
			virtual ~VulkanRenderPass() override;
		public:
			virtual VkRenderPass GetRenderPassHandle();
			uint32_t GetWidth();
			uint32_t GetHeight();
		private:
			VkRenderPass renderPass = nullptr;
			uint32_t width = 0;
			uint32_t height = 0;
		};
	}
}
