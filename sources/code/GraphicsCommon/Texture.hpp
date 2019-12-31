#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct TextureSubBinding {
			const char *shaderLocation;
			uint8_t textureLocation;
			TextureSubBinding() { shaderLocation = ""; textureLocation = 0; };
			TextureSubBinding(const char *_location, uint8_t _target) : shaderLocation(_location), textureLocation(_target) {};
		};

		struct TextureBindingLayoutCreateInfo {
			uint32_t bindingLocation;
			ShaderStageBit stages;
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

		enum class TextureWrapMode : uint8_t {
			Repeat = 0,
			ClampToEdge,
			ClampToBorder,
			MirroredRepeat,
			MirroredClampToEdge
		};

		enum class TextureFilter : uint8_t {
			Nearest = 0,
			Linear,
			NearestMipMapNearest,
			NearestMipMapLinear,
			LinearMipMapNearest,
			LinearMipMapLinear
		};

		struct TextureOptions {
			TextureWrapMode wrap_mode_u = TextureWrapMode::Repeat;
			TextureWrapMode wrap_mode_v = TextureWrapMode::Repeat;
			TextureWrapMode wrap_mode_w = TextureWrapMode::Repeat;
			TextureFilter min_filter = TextureFilter::LinearMipMapLinear;
			TextureFilter mag_filter = TextureFilter::Linear;
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
			SingleTextureBind() : texture(nullptr), address(0) {}
		};

		struct TextureBindingCreateInfo {
			SingleTextureBind *textures;
			uint32_t textureCount;
			TextureBindingLayout *layout;
		};

		class TextureBinding {
		public:
		};
	};
};
