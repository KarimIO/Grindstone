#pragma once

#include <vector>
#include <GL/gl3w.h>

#include <Common/Graphics/Texture.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class Texture : public Grindstone::GraphicsAPI::Texture {
	public:
		Texture(const CreateInfo& ci);
		Texture(const CubemapCreateInfo& ci);
		virtual void RecreateTexture(const CreateInfo& createInfo) override;
		void CreateTexture(const CreateInfo& createInfo);
		void Bind(int i);

		virtual unsigned int GetTexture() const;

		~Texture();

	protected:
		GLuint textureHandle;
		bool isCubemap;

	};
}
