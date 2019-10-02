#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include <map>
#include <vector>

#include "MaterialManager.hpp"

typedef size_t TextureHandler;

struct TextureContainer {
	unsigned short use_count;
	Texture *texture_;
	TextureContainer(Texture *t);
};

class TextureManager {
public:
	//Texture *preloadTexture(std::string path);
	TextureHandler loadTexture(std::string path, TextureOptions options = TextureOptions());
	//Texture *preloadCubemap(std::string path);
	TextureHandler loadCubemap(std::string path, TextureOptions options = TextureOptions());
	//void loadPreloaded();

	Texture *getTexture(TextureHandler handle);
	TextureContainer *getTextureContainer(TextureHandler handle);

	void removeTexture(TextureContainer *container);
	void removeTexture(TextureHandler handle);

	void writeCubemap(std::string path, unsigned char ***data, uint16_t res);

	~TextureManager();
private:
	std::map<std::string, TextureHandler> texture_map_;
	std::vector<TextureContainer> textures_;
	std::vector<TextureHandler> unloaded_;

	TextureHandler error_texture_;
	TextureHandler error_cubemap_;
};

#endif