#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include "GraphicsDLLPointer.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <unordered_map>

class TextureManager {
	std::unordered_map<std::string, Texture *> textureCache;
	std::unordered_map<std::string, Texture *> cubemapCache;
public:
	void ReserveTextures(int n);
	void ReserveCubemaps(int n);

	Texture *LoadTexture(std::string path, PixelScheme scheme);
	Texture *MakeTexture(std::string identifier, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data);
	Texture *MakeWriteTexture(std::string path, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data);
	void WriteTexture(std::string path, uint32_t width, uint32_t height, uint32_t channels, unsigned char *data);

	Texture *LoadCubemap(std::string path, std::string extension, PixelScheme scheme);
	Texture *MakeCubemap(std::string identifier, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data[6]);
	Texture *MakeWriteCubemap(std::string path, std::string extension, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data[6]);
	void WriteCubemap(std::string path, std::string extension, uint32_t width, uint32_t height, uint32_t channels, unsigned char *data[6]);
};

#endif