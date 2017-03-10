#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "TextureManager.h"
void TextureManager::ReserveTextures(int n) {
	textureCache.reserve(n);
}

void TextureManager::ReserveCubemaps(int n) {
	cubemapCache.reserve(n);
}

unsigned char *TextureManager::LoadTextureData(std::string path, PixelScheme scheme, int &texWidth, int &texHeight) {
	Texture *t = pfnCreateTexture();
	int texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		printf("Texture failed to load!: %s \n", path.c_str());
		return NULL;
	}

	return (unsigned char *)pixels;
}

Texture *TextureManager::LoadTexture(std::string path, PixelScheme scheme) {
	std::unordered_map<std::string, Texture *>::const_iterator got = textureCache.find(path);
	if (got != textureCache.end())
		return got->second;

	Texture *t = pfnCreateTexture();
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		printf("Texture failed to load!: %s \n", path.c_str());
		return NULL;
	}

	t->CreateTexture(pixels, scheme, texWidth, texHeight);

	textureCache[path.c_str()] = t;

	stbi_image_free(pixels);

	return t;
}

Texture *TextureManager::LoadCubemap(std::string path, std::string extension, PixelScheme scheme) {
	std::unordered_map<std::string, Texture *>::const_iterator got = cubemapCache.find(path);
	if (got != cubemapCache.end())
		return got->second;

	std::string facePaths[6];
	facePaths[0] = path + "FT" + extension;
	facePaths[1] = path + "BK" + extension;
	facePaths[2] = path + "UP" + extension;
	facePaths[3] = path + "DN" + extension;
	facePaths[4] = path + "RT" + extension;
	facePaths[5] = path + "LF" + extension;

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels[6];
	for (int i = 0; i < 6; i++) {
		pixels[i] = stbi_load(facePaths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels[i]) {
			printf("Texture failed to load!: %s \n", facePaths[i].c_str());
			for (int j = 0; j < i; i++) {
				stbi_image_free(pixels);
			}
			return NULL;
		}
	}
	Texture *t = pfnCreateTexture();
	t->CreateCubemap(pixels, scheme, texWidth, texHeight);
	for (int i = 0; i < 6; i++) {
		stbi_image_free(pixels[i]);
	}

	cubemapCache[facePaths[0].c_str()] = t;

	return t;
}

Texture *TextureManager::MakeTexture(std::string identifier, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data) {
	Texture *t = pfnCreateTexture();

	t->CreateTexture(data, scheme, width, height);
	textureCache[identifier] = t;
	return t;
}

Texture *TextureManager::MakeWriteTexture(std::string path, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data) {
	Texture *t = MakeTexture(path, width, height, channels, scheme, data);
	WriteTexture(path, width, height, channels, data);
	return t;
}

void TextureManager::WriteTexture(std::string path, uint32_t width, uint32_t height, uint32_t channels, unsigned char *data) {
	stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
}

Texture *TextureManager::MakeCubemap(std::string identifier, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data[6]) {
	Texture *t = pfnCreateTexture();

	t->CreateCubemap(data, scheme, width, height);

	cubemapCache[identifier] = t;
	return t;
}

Texture *TextureManager::MakeWriteCubemap(std::string path, std::string extension, uint32_t width, uint32_t height, uint32_t channels, PixelScheme scheme, unsigned char *data[6]) {
	Texture *t = MakeCubemap((path + "FT" + extension), width, height, channels, scheme, data);
	WriteCubemap(path, extension, width, height, channels, data);
	return t;
}

void TextureManager::WriteCubemap(std::string path, std::string extension, uint32_t width, uint32_t height, uint32_t channels, unsigned char *data[6]) {
	std::string facePaths[6];
	facePaths[0] = path + "FT" + extension;
	facePaths[1] = path + "BK" + extension;
	facePaths[2] = path + "UP" + extension;
	facePaths[3] = path + "DN" + extension;
	facePaths[4] = path + "RT" + extension;
	facePaths[5] = path + "LF" + extension;

	for (int i = 0; i < 6; i++)
		stbi_write_png(facePaths[i].c_str(), width, height, channels, data[i], width * channels);
}

void TextureManager::Shutdown() {
	for (const auto& n : textureCache) {
		n.second->Cleanup();
		pfnDeleteGraphicsPointer(n.second);
	}

	for (const auto& n : cubemapCache) {
		n.second->Cleanup();
		pfnDeleteGraphicsPointer(n.second);
	}
}