#pragma once

#include "../GraphicsCommon/Framebuffer.hpp"
#include <d3d12.h>
#include <vector>
#include <stdint.h>


namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12Framebuffer : public Framebuffer {
		public:
			DirectX12Framebuffer(FramebufferCreateInfo ci);
			virtual ~DirectX12Framebuffer() override;
		public:
			// VkFramebuffer getFramebuffer();
		public:
			virtual float getExposure(int i) override;
			virtual void Clear(ClearMode mask) override;
			virtual void CopyFrom(Framebuffer *) override;
			virtual void BindWrite(bool depth) override;
			virtual void BindTextures(int i) override;
			virtual void Bind(bool depth) override;
			virtual void BindRead() override;
			virtual void Unbind() override;
		private:
			// VkFramebuffer framebuffer_;
		};
	}
}