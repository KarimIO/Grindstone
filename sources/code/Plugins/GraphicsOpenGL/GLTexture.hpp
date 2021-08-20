#ifndef _GL_TEXTURE_H
#define _GL_TEXTURE_H

#include "GL/gl3w.h"
#include <Common/Graphics/Texture.hpp>
#include <vector>

namespace Grindstone {
	namespace GraphicsAPI {
		void translateColorFormats(ColorFormat inFormat, bool &isCompressed, GLenum &format, GLint &internalFormat);
		void translateDepthFormats(DepthFormat inFormat, GLenum &format, GLint &internalFormat);

		class GLTexture : public Texture {
			GLuint textureHandle;
			bool isCubemap;
		public:
			GLTexture(CreateInfo& ci);
			GLTexture(CubemapCreateInfo& ci);
			void bind(int i);

			virtual unsigned int getTexture();

			GLenum translateTexWrap(TextureWrapMode);
			GLenum translateTexFilter(TextureFilter);

			~GLTexture();
		};

		class GLTextureBinding : public TextureBinding {
			std::vector<GLTexture *> textures;
			std::vector<uint32_t> targets;
		public:
			GLTextureBinding(CreateInfo& ci);
			void bind();
		};

		class GLTextureBindingLayout : public TextureBindingLayout {
		public:
			TextureSubBinding *subbindings;
			uint32_t subbindingCount;
			GLTextureBindingLayout(CreateInfo& ci);
			TextureSubBinding getSubBinding(uint32_t i);
			uint32_t getNumSubBindings();
		};
	}
}

#endif