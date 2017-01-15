#ifndef _TEXTURE_MANAGER_H

#include "GraphicsDLLPointer.h"
#include <stb_image.h>
#include <stb_image_write.h>

Texture *LoadTexture(std::string path, PixelScheme scheme);
Texture *LoadCubemap(std::string path, std::string extension, PixelScheme scheme);
void WriteTexture(const char *path, uint32_t width, uint32_t height, uint32_t channels, unsigned char *data);

#endif