#pragma once

#include <vector>
#include <GL/gl3w.h>

#include <Common/Graphics/Texture.hpp>

namespace Grindstone::GraphicsAPI {
	class GLTexture : public Texture {
		GLuint textureHandle;
		bool isCubemap;
	public:
		GLTexture(CreateInfo& ci);
		GLTexture(CubemapCreateInfo& ci);
		virtual void RecreateTexture(CreateInfo& createInfo) override;
		void CreateTexture(CreateInfo& createInfo);
		void Bind(int i);

		virtual unsigned int GetTexture();

		~GLTexture();
	};
}
