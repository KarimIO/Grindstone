#pragma once

#include <stdint.h>
#include "Formats.h"

struct TextureSubBinding {
	const char *shaderLocation;
	uint8_t textureLocation;
	TextureSubBinding() { shaderLocation = ""; textureLocation = 0; };
	TextureSubBinding(const char *_location, uint8_t _target) : shaderLocation(_location), textureLocation(_target) {};
};

struct TextureBindingLayoutCreateInfo {
	uint32_t bindingLocation;
	uint32_t stages;
	TextureSubBinding *bindings;
	uint32_t bindingCount;
};

class TextureBindingLayout {
public:
};

struct TextureMipMapCreateInfo {
	unsigned char *data;
	uint32_t size;
	uint32_t width, height;
};

struct TextureCreateInfo {
	unsigned char *data;
	uint32_t width, height;
	uint16_t mipmaps;
	ColorFormat format;
};

struct CubemapCreateInfo {
	unsigned char *data[6];
	uint32_t width, height;
	uint16_t mipmaps;
	ColorFormat format;
};

class Texture {
public:
};

struct SingleTextureBind {
	Texture *texture;
	uint8_t address;
};

struct TextureBindingCreateInfo {
	SingleTextureBind *textures;
	uint32_t textureCount;
};

class TextureBinding {
public:
};