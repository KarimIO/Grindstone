#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <stdint.h>
#include <string>
#include "../GraphicsCommon/GLDefDLL.h"

enum PixelScheme {
	COLOR_R,
	COLOR_RG,
	COLOR_RGB,
	COLOR_RGBA,
	COLOR_SRGB
};

class Texture {
public:
	virtual void CreateTexture(unsigned char *pixels,		PixelScheme scheme, uint32_t width, uint32_t height) = 0;
	virtual void CreateCubemap(unsigned char* pixels[6],	PixelScheme scheme, uint32_t width, uint32_t height) = 0;

	virtual void Bind(int bindTo) = 0;
	virtual void BindCubemap(int bindTo) = 0;
	virtual int GetTextureLocation() = 0;

	virtual void Cleanup() = 0;
};

extern "C" GRAPHICS_EXPORT Texture* createTexture();

#endif