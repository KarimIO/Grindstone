#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include <Common/Graphics/Framebuffer.hpp>
#include <Common/Graphics/Formats.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class RenderPass;

	class Framebuffer : public Grindstone::GraphicsAPI::Framebuffer {
	public:
		Framebuffer(
			RenderPass* renderPass,
			VkFramebuffer framebuffer,
			uint32_t width,
			uint32_t height,
			const char* debugName
		);
		Framebuffer(const CreateInfo& createInfo);
		virtual ~Framebuffer() override;
		void UpdateNativeFramebuffer(
			RenderPass* renderPass,
			VkFramebuffer framebuffer,
			uint32_t width,
			uint32_t height
		);
	public:
		VkFramebuffer GetFramebuffer() const;
	public:
		virtual Grindstone::GraphicsAPI::RenderPass* GetRenderPass() const override;
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
		virtual Grindstone::GraphicsAPI::Image* GetRenderTarget(uint32_t index) const override;
		virtual Grindstone::GraphicsAPI::Image* GetDepthStencilTarget() const override;
	private:
		void Create();
		void Cleanup();

		std::string debugName;
		std::vector<Image*> colorAttachments;
		Image* depthAttachment = nullptr;

		VkFramebuffer framebuffer = nullptr;
		GraphicsAPI::RenderPass* renderPass = nullptr;
		bool isCubemap = false;

		uint32_t width = 0;
		uint32_t height = 0;
	};
}
