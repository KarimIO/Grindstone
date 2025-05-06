#pragma once

#include <stdint.h>
#include <vector>

#include <Common/Graphics/RenderTarget.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class RenderTarget : public Grindstone::GraphicsAPI::RenderTarget {
	public:
		RenderTarget(const CreateInfo& createInfo);
		virtual uint32_t GetHandle() const;
		virtual void Resize(uint32_t width, uint32_t height);

		bool IsCubemap() const;
		virtual void Bind();
		virtual void Bind(uint32_t i);
		virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data);
		virtual ~RenderTarget();

	private:
		bool isCubemap;
		GLint internalFormat;
		GLenum format;
		GLenum type;
		GLuint renderTargetHandle;
		uint32_t width, height;
	};
}
