#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include "Texture.h"
#include "GraphicsWrapper.h"
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <unordered_map>

class TextureManager {
	std::unordered_map<std::string, Texture *> textureCache;
	std::unordered_map<std::string, Texture *> cubemapCache;
public:
	GraphicsWrapper *graphicsWrapper;
	void ReserveTextures(int n);
	void ReserveCubemaps(int n);
	
	Texture *LoadTexture(std::string path);

	void Shutdown();
};

#endif