#pragma once

#include <string>
#include <vector>

#include <Common/Graphics/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(VkRenderPass renderPass, uint32_t width, uint32_t height);
			VulkanRenderPass(RenderPass::CreateInfo& createInfo);
			virtual ~VulkanRenderPass() override;
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual uint32_t GetWidth() override;
			virtual uint32_t GetHeight() override;
		public:
			void Update(VkRenderPass renderPass, uint32_t width, uint32_t height);
			virtual VkRenderPass GetRenderPassHandle();
		private:
			void Create();
			void Cleanup();

			std::string debugName;
			std::vector<ColorFormat> colorFormats;
			DepthFormat depthFormat = DepthFormat::None;
			bool shouldClearDepthOnLoad = true;
			VkRenderPass renderPass = nullptr;
			uint32_t width = 0;
			uint32_t height = 0;
		};
	}
}
