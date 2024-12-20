#pragma once

#include <stdint.h>
#include <vector>

#include <Common/Graphics/RenderTarget.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class RenderTarget : public Grindstone::GraphicsAPI::RenderTarget {
	public:
		RenderTarget(const CreateInfo& createInfoList);
		RenderTarget(const CreateInfo *createInfoList, uint32_t createInfoCount, bool isCcubemap);
		virtual uint32_t GetHandle() const;
		virtual uint32_t GetHandle(uint32_t i) const;
		uint32_t GetNumRenderTargets() const;
		virtual void Resize(uint32_t width, uint32_t height);

		bool IsCubemap() const;
		virtual void Bind();
		virtual void Bind(uint32_t i);
		virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data);
		virtual ~RenderTarget();
	private:
		void CreateRenderTargets();
	private:
		bool isCubemap;
		std::vector<GLint> internalFormats;
		std::vector<GLenum> formats;
		std::vector<GLuint> renderTargetHandles;
		uint32_t renderTargetCount;
		uint32_t width, height;
	};
}
