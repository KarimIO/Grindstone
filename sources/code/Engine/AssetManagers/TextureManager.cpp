// STD headers
#include <fstream>

// My Class
#include "TextureManager.hpp"

// Included Classes
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "GraphicsPipelineManager.hpp"
#include "TextureManager.hpp"
#include "Core/Engine.hpp"
#include "../FormatCommon/DDSformat.hpp"

// Util Classes
/*
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
*/
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
//#define STB_DXT_IMPLEMENTATION
#include <stb/stb_dxt.h>


#include "Core/Utilities.hpp"

#include "../Converter/ImageConverter.hpp"

TextureContainer::TextureContainer(Grindstone::GraphicsAPI::Texture *t) {
	texture_ = t;
}

TextureHandler TextureManager::loadCubemap(std::string path, Grindstone::GraphicsAPI::TextureOptions options) {
	if (texture_map_[path]) {
		return texture_map_[path];
	}

	const char *filecode = path.c_str() + path.size() - 3;

	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL) {
			GRIND_WARN("Cubemap failed to load: {0}", path);
			return -1;
		}

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			fclose(fp);
			GRIND_WARN("Invalid DDS cubemap: {0}", path);
			return -1;
		}

		DDSHeader header;

		fread(&header, 124, 1, fp);

		unsigned char * buffer;
		unsigned int bufsize;
		bufsize = header.dwMipMapCount > 1 ? header.dwPitchOrLinearSize * 2 : header.dwPitchOrLinearSize;
		bufsize *= 6;
		buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
		fread(buffer, 1, bufsize, fp);

		fclose(fp);

		bool alphaflag = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;

		unsigned int components = (header.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
		Grindstone::GraphicsAPI::ColorFormat format;
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = alphaflag ? Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT1 : Grindstone::GraphicsAPI::ColorFormat::RGB_DXT1;
			break;
		case FOURCC_DXT3:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT5;
			break;
		default:
			free(buffer);
			GRIND_WARN("Invalid FourCC in cubemap: {0}", path);
			return -1;
		}

		Grindstone::GraphicsAPI::TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.dwMipMapCount;
		createInfo.format = format;
		createInfo.width = header.dwWidth;
		createInfo.height = header.dwHeight;
		createInfo.ddscube = true;
		createInfo.options = options;

		Grindstone::GraphicsAPI::Texture *t = engine.getGraphicsWrapper()->createTexture(createInfo);
		TextureHandler handle = textures_.size();
		texture_map_[path] = handle;
		textures_.emplace_back(t);

		if (engine.getSettings()->show_texture_load_)
			GRIND_LOG("Cubemap loaded: {0}", path.c_str());

		stbi_image_free(buffer);

		return handle;
	}
	else {
		size_t d = path.find_last_of('.');
		std::string ext = path.substr(d + 1);
		path = path.substr(0, d);

		std::string facePaths[6];
		facePaths[0] = path + "_ft." + ext;
		facePaths[1] = path + "_bk." + ext;
		facePaths[2] = path + "_up." + ext;
		facePaths[3] = path + "_dn." + ext;
		facePaths[4] = path + "_rt." + ext;
		facePaths[5] = path + "_lf." + ext;


		Grindstone::GraphicsAPI::CubemapCreateInfo createInfo;
		 
		int texWidth, texHeight, texChannels;
		for (int i = 0; i < 6; i++) {
			createInfo.data[i] = stbi_load(facePaths[i].c_str(), &texWidth, &texHeight, &texChannels, 4);
			if (!createInfo.data[i]) {
				for (int j = 0; j < i; j++) {
					stbi_image_free(createInfo.data[j]);
				}

				GRIND_WARN("Texture failed to load!: {0}", facePaths[i].c_str());
				return -1;
			}
		}

		GRIND_LOG("Cubemap loaded: {0}", path.c_str());

		Grindstone::GraphicsAPI::ColorFormat format;
		switch (4) {
		case 1:
			format = Grindstone::GraphicsAPI::ColorFormat::R8;
			break;
		case 2:
			format = Grindstone::GraphicsAPI::ColorFormat::R8G8;
			break;
		case 3:
			format = Grindstone::GraphicsAPI::ColorFormat::R8G8B8;
			break;
		default:
		case 4:
			format = Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8;
			break;
		}

		createInfo.format = format;
		createInfo.mipmaps = 0;
		createInfo.width = texWidth;
		createInfo.height = texHeight;
		createInfo.options = options;

		Grindstone::GraphicsAPI::Texture *t = engine.getGraphicsWrapper()->createCubemap(createInfo);
		TextureHandler handle = textures_.size();
		textures_.push_back(TextureContainer(t));
		texture_map_[path] = handle;

		for (int i = 0; i < 6; i++) {
			stbi_image_free(createInfo.data[i]);
		}

		return handle;
	}
}

TextureHandler TextureManager::loadTexture(std::string path, Grindstone::GraphicsAPI::TextureOptions options) {
	if (texture_map_[path]) {
		return texture_map_[path];
	}

	const char *filecode = path.c_str() + path.size() - 3;
	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL) {
			GRIND_WARN("Texture failed to load!: {0}", path.c_str());
			return 0;
		}

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			GRIND_ERROR("Texture failed to load!: {0}", path.c_str());
			fclose(fp);
			return 0;
		}

		DDSHeader header;

		fread(&header, 124, 1, fp);

		unsigned char * buffer;
		unsigned int bufsize;
		bufsize = header.dwMipMapCount > 1 ? header.dwPitchOrLinearSize * 2 : header.dwPitchOrLinearSize;
		buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
		fread(buffer, 1, bufsize, fp);

		fclose(fp);

		bool alphaflag = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;

		unsigned int components = (header.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
		Grindstone::GraphicsAPI::ColorFormat format;
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT1; // alphaflag ? Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT1 : Grindstone::GraphicsAPI::ColorFormat::RGB_DXT1;
			break;
		case FOURCC_DXT3:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = Grindstone::GraphicsAPI::ColorFormat::RGBA_DXT5;
			break;
		default:
			free(buffer);
			GRIND_WARN("Invalid FourCC in texture: {0}", path.c_str());
			return error_texture_;
		}

		Grindstone::GraphicsAPI::TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.dwMipMapCount;
		createInfo.format = format;
		createInfo.width = header.dwWidth;
		createInfo.height = header.dwHeight;
		createInfo.ddscube = false;
		createInfo.options = options;

		Grindstone::GraphicsAPI::Texture *t = nullptr;
		try {
			t = engine.getGraphicsWrapper()->createTexture(createInfo);
		}
		catch (const char *e) {
			GRIND_ERROR("{0}", e);
		}
		TextureHandler handle = textures_.size();
		texture_map_[path] = handle;
		textures_.emplace_back(t);

		if (engine.getSettings()->show_texture_load_)
			GRIND_LOG("Texture loaded: {0}", path.c_str());

		stbi_image_free(buffer);

		return handle;
	}
	else {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		texChannels = 4;
		if (!pixels) {
			GRIND_WARN("Texture failed to load: {0}", path);
			return error_texture_;
		}

		Grindstone::GraphicsAPI::ColorFormat format;
		switch (texChannels) {
		case 1:
			format = Grindstone::GraphicsAPI::ColorFormat::R8;
			break;
		case 2:
			format = Grindstone::GraphicsAPI::ColorFormat::R8G8;
			break;
		case 3:
			format = Grindstone::GraphicsAPI::ColorFormat::R8G8B8;
			break;
		default:
		case 4:
			format = Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8;
			break;
		}

		Grindstone::GraphicsAPI::TextureCreateInfo createInfo;
		createInfo.format = format;
		createInfo.mipmaps = 0;
		createInfo.data = pixels;
		createInfo.width = texWidth;
		createInfo.height = texHeight;
		createInfo.ddscube = false;
		createInfo.options = options;

		Grindstone::GraphicsAPI::Texture *t = engine.getGraphicsWrapper()->createTexture(createInfo);
		TextureHandler handle = textures_.size();
		textures_.emplace_back(TextureContainer(t));
		texture_map_[path] = handle;

		stbi_image_free(pixels);

		if (engine.getSettings()->show_texture_load_)
			GRIND_LOG("Texture loaded: {0}", path.c_str());

		return handle;
	}
	return error_texture_;
}

TextureContainer *TextureManager::getTextureContainer(TextureHandler handle) {
	return &textures_[handle];
}

void TextureManager::reloadAll() {
	for (size_t i = 0; i < textures_.size(); ++i) {
		reloadTexture(i);
	}
}

void TextureManager::reloadTexture(TextureHandler handle) {
	engine.getGraphicsWrapper()->deleteTexture(textures_[handle].texture_);
}

Grindstone::GraphicsAPI::Texture *TextureManager::getTexture(TextureHandler handle) {
	return getTextureContainer(handle)->texture_;
}

void TextureManager::removeTexture(TextureContainer *container) {
	if (container->use_count > 1) {
		container->use_count -= 1;
	}
	else {
		engine.getGraphicsWrapper()->deleteTexture(container->texture_);
		// TODO: remove from array.
	}
}

void TextureManager::removeTexture(TextureHandler handle) {
	TextureContainer *texture_container = getTextureContainer(handle);
	removeTexture(texture_container);
}

void TextureManager::writeCubemap(std::string path, unsigned char ***data, uint16_t res) {
	const char *filecode = path.c_str() + path.size() - 3;

	if (strncmp(filecode, "dds", 3) == 0) {
		ConvertBC123(data, true, res, res, C_BC1, path, false);
	}
	else {
		size_t d = path.find_last_of('.');
		std::string ext = path.substr(d + 1);
		path = path.substr(0, d);

		std::string facePaths[6];
		facePaths[0] = path + "_ft." + ext;
		facePaths[1] = path + "_bk." + ext;
		facePaths[2] = path + "_up." + ext;
		facePaths[3] = path + "_dn." + ext;
		facePaths[4] = path + "_rt." + ext;
		facePaths[5] = path + "_lf." + ext;

		for (int i = 0; i < 6; ++i) {
			stbi_write_png(facePaths[i].c_str(), res, res, 4, data[i], res * 4);
		}
	}
}

void TextureManager::cleanup() {
	for (auto &texture : textures_) {
		removeTexture(&texture);
	}

	texture_map_.clear();
}

TextureManager::~TextureManager() {
	cleanup();
}