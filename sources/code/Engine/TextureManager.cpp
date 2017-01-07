#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

Texture *LoadTexture(std::string path, PixelScheme scheme) {
	Texture *t = pfnCreateTexture();
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	t->CreateTexture(pixels, scheme, texWidth, texHeight);

	if (!pixels)
		printf("Texture failed to load!: %s \n", path.c_str());

	return t;
}

Texture *LoadCubemap(std::string path, std::string extension, PixelScheme scheme) {
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
	}
	Texture *t = pfnCreateTexture();
	t->CreateCubemap(pixels, scheme, texWidth, texHeight);
	for (int i = 0; i < 6; i++) {
		stbi_image_free(pixels[i]);
	}
	return t;
}

void WriteTexture(const char *path, uint32_t width, uint32_t height, uint32_t channels, unsigned char *data) {
	stbi_write_png(path, width, height, channels, data, width * channels);
}
