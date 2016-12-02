#ifndef _GL_TEXTURE_H
#define _GL_TEXTURE_H

#include "../GraphicsCommon/Texture.h"
#include "../GraphicsCommon/GLDefDLL.h"

class GLTexture : public Texture {
public:
	unsigned int textureID;
	virtual void CreateTexture(unsigned char *pixels,		PixelScheme scheme, uint32_t width, uint32_t height);
	virtual void CreateCubemap(unsigned char* pixels[],	PixelScheme scheme, uint32_t width, uint32_t height);

	virtual void Bind();
	virtual int GetTextureLocation();

	virtual void Cleanup();
};

#endif