#pragma once

#include <string>
#include <vector>

#include <Common/Graphics/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(VkRenderPass renderPass, const char* renderPassName);
			VulkanRenderPass(RenderPass::CreateInfo& createInfo);
			virtual ~VulkanRenderPass() override;
		public:
			void Update(VkRenderPass renderPass);
			virtual VkRenderPass GetRenderPassHandle() const;
			virtual const char* GetDebugName() const override;
			virtual const float* GetDebugColor() const override;

		private:
			void Create();
			void Cleanup();

			std::string debugName;
			float debugColor[4] = {};
			std::vector<ColorFormat> colorFormats;
			DepthFormat depthFormat = DepthFormat::None;
			bool shouldClearDepthOnLoad = true;
			VkRenderPass renderPass = nullptr;
		};
	}
}
