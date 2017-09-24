#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "TextureManager.h"
void TextureManager::ReserveTextures(int n) {
	textureCache.reserve(n);
}

void TextureManager::ReserveCubemaps(int n) {
	cubemapCache.reserve(n);
}

Texture * TextureManager::LoadTexture(std::string path) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_default);

	if (!pixels) {
		printf("Texture failed to load!: %s \n", path.c_str());
		return NULL;
	}
	
	TextureCreateInfo createInfo;
	createInfo.data = pixels;
	createInfo.width = texWidth;
	createInfo.height = texHeight;

	switch (texChannels) {
	case 1:
		createInfo.format = FORMAT_COLOR_R8;
		break;
	case 2:
		createInfo.format = FORMAT_COLOR_R8G8;
		break;
	case 3:
		createInfo.format = FORMAT_COLOR_R8G8B8;
		break;
	default:
	case 4:
		createInfo.format = FORMAT_COLOR_R8G8B8A8;
		break;
	}

	Texture *t = graphicsWrapper->CreateTexture(createInfo);
	textureCache[path] = t;

	stbi_image_free(pixels);

	return t;
}


void TextureManager::Shutdown() {
	for (const auto& n : textureCache) {
		//n.second->Cleanup();
		//pfnDeleteGraphicsPointer(n.second);
	}

	for (const auto& n : cubemapCache) {
		//n.second->Cleanup();
		//pfnDeleteGraphicsPointer(n.second);
	}
}