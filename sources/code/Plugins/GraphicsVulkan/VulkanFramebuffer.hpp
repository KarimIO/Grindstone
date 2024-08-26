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
			VulkanFramebuffer(
				RenderPass* renderPass,
				VkFramebuffer framebuffer,
				uint32_t width,
				uint32_t height,
				const char* debugName
			);
			VulkanFramebuffer(Framebuffer::CreateInfo& createInfo);
			virtual ~VulkanFramebuffer() override;
			void UpdateNativeFramebuffer(
				RenderPass* renderPass,
				VkFramebuffer framebuffer,
				uint32_t width,
				uint32_t height
			);
		public:
			VkFramebuffer GetFramebuffer() const;
		public:
			virtual uint32_t GetAttachment(uint32_t attachmentIndex) override;
			virtual RenderPass* GetRenderPass() const override;
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void Clear(ClearMode mask) override;
			virtual void BindTextures(int i) override;
			virtual void Bind() override;
			virtual void BindWrite() override;
			virtual void BindRead() override;
			virtual void Unbind() override;
			virtual uint32_t GetWidth() const override;
			virtual uint32_t GetHeight() const override;
			virtual uint32_t GetRenderTargetCount() const override;
			virtual RenderTarget* GetRenderTarget(uint32_t index) const override;
			virtual DepthTarget* GetDepthTarget() const override;
		private:
			void Create();
			void Cleanup();

			std::string debugName;
			std::vector<VulkanRenderTarget*> colorAttachments;
			VulkanDepthTarget* depthAttachment = nullptr;

			VkFramebuffer framebuffer = nullptr;
			RenderPass* renderPass = nullptr;
			bool isCubemap = false;

			uint32_t width = 0;
			uint32_t height = 0;
		};
	}
}
