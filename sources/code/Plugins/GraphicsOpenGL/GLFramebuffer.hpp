#pragma once

#include <Common/Graphics/Framebuffer.hpp>
#include <Common/Graphics/DLLDefs.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class Framebuffer : public Grindstone::GraphicsAPI::Framebuffer {
	public:
		Framebuffer(const CreateInfo& createInfo);
		~Framebuffer();

		virtual void Blit(uint32_t i, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

		// Inherited via Framebuffer
		virtual RenderPass* GetRenderPass() const override;
		virtual uint32_t GetAttachment(uint32_t attachmentIndex) override;
		virtual void Clear(ClearMode mask) override;
		virtual void Resize(uint32_t newWidth, uint32_t newHeight) override;
		virtual void Bind() override;
		virtual void BindWrite() override;
		virtual void BindRead() override;
		virtual void BindTextures(int i) override;
		virtual void Unbind() override;
		virtual uint32_t GetWidth() const override;
		virtual uint32_t GetHeight() const override;
		virtual uint32_t GetRenderTargetCount() const override;
		virtual Grindstone::GraphicsAPI::RenderTarget* GetRenderTarget(uint32_t index) const override;
		virtual Grindstone::GraphicsAPI::DepthTarget* GetDepthTarget() const override;
	private:
		void CreateFramebuffer();
	private:
		std::string debugName;
		GLuint framebuffer = 0;
		GLsizei numTotalRenderTargets = 0;
		Grindstone::GraphicsAPI::RenderPass* renderPass = nullptr;
		std::vector<Grindstone::GraphicsAPI::RenderTarget*> colorAttachments;
		Grindstone::GraphicsAPI::DepthTarget *depthTarget = nullptr;
		uint32_t width;
		uint32_t height;
	};
}
