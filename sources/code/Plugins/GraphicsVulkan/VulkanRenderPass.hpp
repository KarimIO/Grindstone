#pragma once

#include <string>
#include <vector>

#include <Common/Graphics/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(VkRenderPass renderPass);
			VulkanRenderPass(RenderPass::CreateInfo& createInfo);
			virtual ~VulkanRenderPass() override;
		public:
			void Update(VkRenderPass renderPass);
			virtual VkRenderPass GetRenderPassHandle() const;
		private:
			void Create();
			void Cleanup();

			std::string debugName;
			std::vector<ColorFormat> colorFormats;
			DepthFormat depthFormat = DepthFormat::None;
			bool shouldClearDepthOnLoad = true;
			VkRenderPass renderPass = nullptr;
		};
	}
}
