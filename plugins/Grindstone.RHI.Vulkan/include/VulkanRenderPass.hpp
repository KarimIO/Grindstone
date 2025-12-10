#pragma once

#include <string>
#include <vector>

#include <Common/Graphics/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class RenderPass : public Grindstone::GraphicsAPI::RenderPass {
	public:
		RenderPass(VkRenderPass renderPass, const char* renderPassName);
		RenderPass(const CreateInfo& createInfo);
		virtual ~RenderPass() override;
	public:
		void Update(VkRenderPass renderPass);
		virtual VkRenderPass GetRenderPassHandle() const;
		virtual const char* GetDebugName() const override;
		virtual const float* GetDebugColor() const override;
		size_t GetColorAttachmentCount() const;
		VkFormat GetVkColorFormat(size_t i) const;
		VkFormat GetVkDepthFormat() const;
		VkFormat GetVkStencilFormat() const;

		bool shouldClearDepthOnLoad = true;
	private:
		void Create();
		void Cleanup();

		std::string debugName;
		float debugColor[4] = {};
		std::vector<RenderPass::AttachmentInfo> colorAttachments;
		Format depthFormat = Format::Invalid;
		bool ownsRenderPass = true;
		VkRenderPass renderPass = nullptr;
	};
}
