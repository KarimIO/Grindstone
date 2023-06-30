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
			virtual void RecreateTexture(CreateInfo& createInfo) override;
			void CreateTexture(CreateInfo& createInfo);
			void Bind(int i);

			virtual unsigned int GetTexture();

			GLenum TranslateTexWrap(TextureWrapMode);
			GLenum TranslateTexFilter(TextureFilter);

			~GLTexture();
		};
	}
}

#endif
