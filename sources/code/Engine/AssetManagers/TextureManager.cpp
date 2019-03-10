// STD headers
#include <fstream>

// My Class
#include "TextureManager.hpp"

// Included Classes
#include "GraphicsWrapper.hpp"
#include "GraphicsPipelineManager.hpp"
#include "TextureManager.hpp"
#include "Core/Engine.hpp"
#include "../FormatCommon/DDSformat.hpp"

// Util Classes
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_DXT_IMPLEMENTATION
#include <stb/stb_dxt.h>

#include "../Utilities/Logger.hpp"
#include "Core/Utilities.hpp"

TextureContainer::TextureContainer(Texture *t) {
	texture_ = t;
}	

TextureHandler TextureManager::loadCubemap(std::string path, TextureOptions options) {
	if (texture_map_[path]) {
		return texture_map_[path];
	}

	const char *filecode = path.c_str() + path.size() - 3;

	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL) {
			std::string warn = "Cubemap failed to load: " + path + " \n";
			LOG_WARN(warn);
			return -1;
		}

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			fclose(fp);
			LOG_WARN("Invalid DDS cubemap: " + path + " \n");
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
		if (alphaflag)
			LOG_WARN(path + "\n");

		unsigned int components = (header.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
		ColorFormat format;
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = alphaflag ? FORMAT_COLOR_RGBA_DXT1 : FORMAT_COLOR_RGB_DXT1;
			break;
		case FOURCC_DXT3:
			format = FORMAT_COLOR_RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = FORMAT_COLOR_RGBA_DXT5;
			break;
		default:
			free(buffer);
			LOG_WARN("Invalid FourCC in cubemap: %s \n", path);
			return -1;
		}

		TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.dwMipMapCount;
		createInfo.format = format;
		createInfo.width = header.dwWidth;
		createInfo.height = header.dwHeight;
		createInfo.ddscube = true;
		createInfo.options = options;

		Texture *t = engine.getGraphicsWrapper()->CreateTexture(createInfo);
		TextureHandler handle = textures_.size();
		texture_map_[path] = handle;
		textures_.emplace_back(t);

		if (engine.getSettings()->show_texture_load_)
			LOG("Cubemap loaded: %s \n", path.c_str());

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


		CubemapCreateInfo createInfo;

		int texWidth, texHeight, texChannels;
		for (int i = 0; i < 6; i++) {
			createInfo.data[i] = stbi_load(facePaths[i].c_str(), &texWidth, &texHeight, &texChannels, 4);
			if (!createInfo.data[i]) {
				for (int j = 0; j < i; j++) {
					stbi_image_free(createInfo.data[j]);
				}

				LOG_WARN("Texture failed to load!: %s\n");
				return -1;
			}
		}

		LOG("Cubemap loaded: %s \n", path.c_str());

		ColorFormat format;
		switch (4) {
		case 1:
			format = FORMAT_COLOR_R8;
			break;
		case 2:
			format = FORMAT_COLOR_R8G8;
			break;
		case 3:
			format = FORMAT_COLOR_R8G8B8;
			break;
		default:
		case 4:
			format = FORMAT_COLOR_R8G8B8A8;
			break;
		}

		createInfo.format = format;
		createInfo.mipmaps = 0;
		createInfo.width = texWidth;
		createInfo.height = texHeight;
		createInfo.options = options;

		Texture *t = engine.getGraphicsWrapper()->CreateCubemap(createInfo);
		TextureHandler handle = textures_.size();
		textures_.push_back(TextureContainer(t));
		texture_map_[path] = handle;

		for (int i = 0; i < 6; i++) {
			stbi_image_free(createInfo.data[i]);
		}

		return handle;
	}
}

TextureHandler TextureManager::loadTexture(std::string path, TextureOptions options) {
	if (texture_map_[path]) {
		return texture_map_[path];
	}

	const char *filecode = path.c_str() + path.size() - 3;
	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL) {
			LOG_WARN("Texture failed to load!: %s \n", path.c_str());
			return 0;
		}

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			printf("Texture failed to load!: %s \n", path.c_str());
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

		bool alphaflag = 0; // header.ddspf.dwFlags & DDPF_ALPHAPIXELS;
		//if (alphaflag)
		//	LOG_WARN(path + "\n");

		unsigned int components = (header.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
		ColorFormat format;
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = FORMAT_COLOR_RGBA_DXT1; // alphaflag ? FORMAT_COLOR_RGBA_DXT1 : FORMAT_COLOR_RGB_DXT1;
			break;
		case FOURCC_DXT3:
			format = FORMAT_COLOR_RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = FORMAT_COLOR_RGBA_DXT5;
			break;
		default:
			free(buffer);
			LOG_WARN("Invalid FourCC in texture: %s \n", path.c_str());
			return error_texture_;
		}

		TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.dwMipMapCount;
		createInfo.format = format;
		createInfo.width = header.dwWidth;
		createInfo.height = header.dwHeight;
		createInfo.ddscube = false;
		createInfo.options = options;

		Texture *t = nullptr;
		try {
			t = engine.getGraphicsWrapper()->CreateTexture(createInfo);
		}
		catch (const char *e) {
			std::cerr << e << "\n";
		}
		TextureHandler handle = textures_.size();
		texture_map_[path] = handle;
		textures_.emplace_back(t);

		if (engine.getSettings()->show_texture_load_)
			LOG("Texture loaded: %s \n", path.c_str());

		stbi_image_free(buffer);

		return handle;
	}
	else {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		texChannels = 4;
		if (!pixels) {
			LOG_WARN("Texture failed to load: %s!\n", path);
			return error_texture_;
		}

		ColorFormat format;
		switch (texChannels) {
		case 1:
			format = FORMAT_COLOR_R8;
			break;
		case 2:
			format = FORMAT_COLOR_R8G8;
			break;
		case 3:
			format = FORMAT_COLOR_R8G8B8;
			break;
		default:
		case 4:
			format = FORMAT_COLOR_R8G8B8A8;
			break;
		}

		TextureCreateInfo createInfo;
		createInfo.format = format;
		createInfo.mipmaps = 0;
		createInfo.data = pixels;
		createInfo.width = texWidth;
		createInfo.height = texHeight;
		createInfo.ddscube = false;
		createInfo.options = options;

		Texture *t = engine.getGraphicsWrapper()->CreateTexture(createInfo);
		TextureHandler handle = textures_.size();
		textures_.emplace_back(TextureContainer(t));
		texture_map_[path] = handle;

		stbi_image_free(pixels);

		if (engine.getSettings()->show_texture_load_)
			LOG("Texture loaded: %s \n", path.c_str());

		return handle;
	}
	return error_texture_;
}

TextureContainer *TextureManager::getTextureContainer(TextureHandler handle) {
	return &textures_[handle];
}

Texture *TextureManager::getTexture(TextureHandler handle) {
	return getTextureContainer(handle)->texture_;
}

void TextureManager::removeTexture(TextureContainer *container) {
	if (container->use_count > 1) {
		container->use_count -= 1;
	}
	else {
		engine.getGraphicsWrapper()->DeleteTexture(container->texture_);
		// TODO: remove from array.
	}
}

void TextureManager::removeTexture(TextureHandler handle) {
	TextureContainer *texture_container = getTextureContainer(handle);
	removeTexture(texture_container);
}

void ExtractBlock(const unsigned char* inPtr, unsigned int width, unsigned char* colorBlock) {
	for (int j = 0; j < 4; j++) {
		memcpy(&colorBlock[j * 4 * 4], inPtr, 4 * 4);
		inPtr += width * 4;
	}
}

unsigned char *CreateMip(unsigned char *pixel, int width, int height) {
	int size = width * height;
	unsigned char *mip = new unsigned char[size * 4];
	int dst = -1;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int src = (i * width * 4 + j * 2) * 4;
			mip[++dst] = pixel[src] / 4;
			mip[dst] += pixel[src + 4] / 4;
			mip[dst] += pixel[src + 8] / 4;
			mip[dst] += pixel[src + 12] / 4;

			mip[++dst] = pixel[src + 1] / 4;
			mip[dst] += pixel[src + 5] / 4;
			mip[dst] += pixel[src + 9] / 4;
			mip[dst] += pixel[src + 13] / 4;

			mip[++dst] = pixel[src + 2] / 4;
			mip[dst] += pixel[src + 6] / 4;
			mip[dst] += pixel[src + 10] / 4;
			mip[dst] += pixel[src + 14] / 4;

			mip[++dst] = pixel[src + 3] / 4;
			mip[dst] += pixel[src + 7] / 4;
			mip[dst] += pixel[src + 11] / 4;
			mip[dst] += pixel[src + 15] / 4;
		}
	}

	return mip;
}

void ConvertBC123(unsigned char **pixels, bool is_cubemap, int width, int height, Compression compression, std::string path) {
	// No Alpha
	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE | DDSD_MIPMAPCOUNT;
	outHeader.dwHeight = height;
	outHeader.dwWidth = width;
	outHeader.dwDepth = 0;
	outHeader.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	outHeader.dwMipMapCount = std::log2(width) + 1;
	if (is_cubemap)
		outHeader.dwCaps2 = DDS_CUBEMAP_ALLFACES;

	bool alpha = false;
	switch (compression) {
	default:
	case C_BC1: {
		outHeader.dwPitchOrLinearSize = width * height / 2;
		outHeader.ddspf.dwFourCC = FOURCC_DXT1;
		break;
	}
	case C_BC2:
		outHeader.dwPitchOrLinearSize = width * height;
		outHeader.ddspf.dwFourCC = FOURCC_DXT3;
		break;
	case C_BC3:
		outHeader.dwPitchOrLinearSize = width * height;
		outHeader.ddspf.dwFourCC = FOURCC_DXT5;
		break;
	}
	char mark[] = { 'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E' };
	std::memcpy(&outHeader.dwReserved1, mark, sizeof(mark));

	bool useMip = outHeader.dwMipMapCount > 1;
	int size = outHeader.dwPitchOrLinearSize;
	int mipsize = size;
	unsigned int blockSize = (outHeader.ddspf.dwFourCC == FOURCC_DXT1) ? 8 : 16;
	for (int i = 1; i < outHeader.dwMipMapCount; i++) {
		mipsize = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		size += mipsize;
	}

	if (is_cubemap)
		size *= 6;

	unsigned char *outData = new unsigned char[size];
	int offset = 0;
	unsigned char block[64];

	int minlev = outHeader.dwMipMapCount - 2;
	int num_faces = is_cubemap ? 6 : 1;
	for (int l = 0; l < num_faces; l++) {
		unsigned char *mip = pixels[l];
		width = outHeader.dwWidth;
		height = outHeader.dwHeight;
		for (int k = 0; k < minlev; k++) {
			for (int j = 0; j < height; j += 4) {
				unsigned char *ptr = mip + j * width * 4;
				for (int i = 0; i < width; i += 4) {
					ExtractBlock(ptr, width, block);
					stb_compress_dxt_block(&outData[offset], block, false, STB_DXT_NORMAL);
					ptr += 4 * 4;
					offset += 8;
				}
			}
			width /= 2;
			height /= 2;

			unsigned char *temp_mip = mip;

			if (k - 1 != minlev)
				mip = CreateMip(temp_mip, width, height);

			if (k != 0)
				delete[] temp_mip;
		}

		memcpy(&outData[offset], &outData[offset], 8); // 2x2
		offset += 8;
		memcpy(&outData[offset], &outData[offset], 8); // 1x1
		offset += 8;
	}

	std::ofstream out(path, std::ios::binary);
	if (out.fail()) {
		std::cerr << "Failed to output to " + path + ".\n";
		return;
	}
	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char *)&filecode, sizeof(char) * 4);
	out.write((const char *)&outHeader, sizeof(outHeader));
	out.write((const char *)outData, size);
	out.close();

	delete[] outData;
}

void TextureManager::writeCubemap(std::string path, unsigned char *data[6], uint16_t res) {
	const char *filecode = path.c_str() + path.size() - 3;

	if (strncmp(filecode, "dds", 3) == 0) {
		ConvertBC123(data, true, res, res, C_BC1, path);
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

TextureManager::~TextureManager() {
	for (auto &texture : textures_) {
		removeTexture(&texture);
	}
}