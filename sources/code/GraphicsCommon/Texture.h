#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <stdint.h>
#include <string>
#include "../GraphicsCommon/GLDefDLL.h"

class Texture {
public:
	virtual void CreateTexture(unsigned char *pixels, uint32_t width, uint32_t height) = 0;
	virtual void CreateCubemap((unsigned char *)pixels[6], uint32_t width, uint32_t height) = 0;

	virtual void Cleanup() = 0;
};

extern "C" GRAPHICS_EXPORT Texture* createTexture();

#endif