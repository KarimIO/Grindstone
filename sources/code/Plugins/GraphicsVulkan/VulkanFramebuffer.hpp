#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include <Common/Graphics/Framebuffer.hpp>
#include <Common/Graphics/Formats.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanRenderTarget;
		class VulkanDepthTarget;

		class VulkanFramebuffer : public Framebuffer {
		public:
			VulkanFramebuffer(RenderPass* renderPass, VkFramebuffer framebuffer);
			VulkanFramebuffer(Framebuffer::CreateInfo& createInfo);
			virtual ~VulkanFramebuffer() override;
		public:
			VkFramebuffer GetFramebuffer();
		public:
			virtual uint32_t GetAttachment(uint32_t attachmentIndex) override;
			virtual RenderPass* GetRenderPass() override;
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void Clear(ClearMode mask) override;
			virtual void BindTextures(int i) override;
			virtual void Bind() override;
			virtual void BindWrite() override;
			virtual void BindRead() override;
			virtual void Unbind() override;
		private:
			void Create();
			void Cleanup();

			std::string debugName;
			std::vector<VulkanRenderTarget*> colorAttachments;
			VulkanDepthTarget* depthAttachment = nullptr;

			VkFramebuffer framebuffer = nullptr;
			RenderPass* renderPass = nullptr;
			bool isCubemap = false;
		};
	}
}
