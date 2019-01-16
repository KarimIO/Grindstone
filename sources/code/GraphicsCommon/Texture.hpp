#pragma once

#include <stdint.h>
#include "Formats.hpp"

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

enum TextureWrapMode {
	TEXWRAP_REPEAT = 0,
	TEXWRAP_CLAMP_TO_EDGE,
	TEXWRAP_CLAMP_TO_BORDER,
	TEXWRAP_MIRRORED_REPEAT,
	TEXWRAP_MIRROR_CLAMP_TO_EDGE
};

enum TextureFilter {
	TEXFILTER_NEAREST = 0,
	TEXFILTER_LINEAR,
	TEXFILTER_NEAREST_MIPMAP_NEAREST,
	TEXFILTER_LINEAR_MIPMAP_NEAREST,
	TEXFILTER_NEAREST_MIPMAP_LINEAR,
	TEXFILTER_LINEAR_MIPMAP_LINEAR,
};

struct TextureOptions {
	TextureWrapMode wrap_mode_u = TEXWRAP_REPEAT;
	TextureWrapMode wrap_mode_v = TEXWRAP_REPEAT;
	TextureWrapMode wrap_mode_w = TEXWRAP_REPEAT;
	TextureFilter min_filter = TEXFILTER_LINEAR_MIPMAP_LINEAR;
	TextureFilter mag_filter = TEXFILTER_LINEAR;
	bool generate_mipmaps = true;
};

struct TextureCreateInfo {
	unsigned char *data;
	uint32_t width, height;
	uint16_t mipmaps;
	bool ddscube;
	ColorFormat format;
	TextureOptions options;
};

struct CubemapCreateInfo {
	unsigned char *data[6];
	uint32_t width, height;
	uint16_t mipmaps;
	ColorFormat format;
	TextureOptions options;
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
	TextureBindingLayout *layout;
};

class TextureBinding {
public:
};