#include "SCubemap.hpp"
#include <iostream>

#include "../Core/Engine.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_DXT_IMPLEMENTATION
#include <stb/stb_dxt.h>

#include <fstream>

#include "DDSformat.hpp"
#include <cstring>

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
			mip[++dst]	= pixel[src] / 4;
			mip[dst]	+= pixel[src + 4] / 4;
			mip[dst]	+= pixel[src + 8] / 4;
			mip[dst]	+= pixel[src + 12] / 4;

			mip[++dst]  = pixel[src + 1] / 4;
			mip[dst]  += pixel[src + 5] / 4;
			mip[dst]  += pixel[src + 9] / 4;
			mip[dst]  += pixel[src + 13] / 4;

			mip[++dst]  = pixel[src + 2] / 4;
			mip[dst]  += pixel[src + 6] / 4;
			mip[dst]  += pixel[src + 10] / 4;
			mip[dst]  += pixel[src + 14] / 4;

			mip[++dst]  = pixel[src + 3] / 4;
			mip[dst]  += pixel[src + 7] / 4;
			mip[dst]  += pixel[src + 11] / 4;
			mip[dst]  += pixel[src + 15] / 4;
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
	outHeader.dwMipMapCount = std::log2(width)+1;
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
	char mark[] = {'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E'};
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
			for (int j = 0; j < height; j+=4) {
				unsigned char *ptr = mip + j * width * 4;
				for (int i = 0; i < width; i+=4) {
					ExtractBlock(ptr, width, block);
					stb_compress_dxt_block(&outData[offset], block, false, STB_DXT_NORMAL);
					ptr += 4 * 4;
					offset += 8;
				}
			}
			width /= 2;
			height /= 2;
			
			unsigned char *temp_mip = mip;

			if (k-1 != minlev)
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

void CubemapSystem::CaptureCubemaps(double) {
	std::cout << "Capture Cubemaps" << "\n";

	glm::mat4 proj = glm::perspective(1.5708f, 1.0f, 0.1f, 1000.0f);
	glm::mat4 view;

	int res = 128;


	if (!engine.settings.debugNoLighting && engine.settings.enableShadows)
		engine.lightSystem.DrawShadows();

	bool shouldUseShadows = engine.settings.enableShadows;
	engine.settings.enableShadows = false;

	int num_passes = 3;
	std::string base_path = "../assets/cubemaps/" + engine.level_file_name_ + "_";

	unsigned char **data = new unsigned char *[6];
	for (int i = 0; i < 6; i++) {
		data[i] = new unsigned char [res * res * 4];
	}

	for (int k = 0; k < num_passes; ++k) {
		for (size_t i = 0; i < components.size(); ++i) {

			std::string path = base_path + std::to_string(i);
			for (size_t j = 0; j < 6; ++j) {
				//materialManager.resetDraws();
				//geometry_system.Cull(cam);
				view = glm::lookAt(components[i].position, components[i].position + gCubeDirections[j].Target, gCubeDirections[j].Up);

				engine.deffUBOBuffer.eyePos.x = components[i].position.x;
				engine.deffUBOBuffer.eyePos.y = components[i].position.y;
				engine.deffUBOBuffer.eyePos.z = components[i].position.z;
				engine.deffUBOBuffer.invProj = glm::inverse(proj);
				engine.deffUBOBuffer.view = glm::inverse(view);

				engine.deffUBOBuffer.resolution.x = res;
				engine.deffUBOBuffer.resolution.y = res;
				engine.deffUBO->UpdateUniformBuffer(&engine.deffUBOBuffer);

				view = proj * view;
				engine.ubo->UpdateUniformBuffer(&view);
				engine.ubo->Bind();

				engine.ubo2->Bind();

				engine.Render();
				engine.graphics_wrapper_->BindDefaultFramebuffer(true);
				engine.gbuffer_images_->RenderScreen(0, res, res, data[j]);
			}

			ConvertBC123(data, true, res, res, C_BC1, path + ".dds");
		}
		engine.settings.enableShadows = true;
		LoadCubemaps();
	}


	for (int i = 0; i < 6; i++) {
		delete[] data[i];
	}
	delete[] data;

	engine.settings.enableShadows = shouldUseShadows;
}

void CubemapSystem::LoadCubemaps() {
	G_NOTIFY("Loading Cubemaps.\n");
	std::string path = "../assets/cubemaps/" + engine.level_file_name_ + "_";
	for (size_t i = 0; i < components.size(); i++) {
		std::string sub_path = path + std::to_string(i) + ".dds";
		try {
			components[i].cubemap = engine.materialManager.LoadCubemap(sub_path);

			SingleTextureBind stb;
			stb.texture = components[i].cubemap;
			stb.address = 4;

			TextureBindingCreateInfo ci;
			ci.textures = &stb;
			ci.layout = engine.reflection_cubemap_layout_;
			ci.textureCount = 1;
			components[i].cubemap_binding = engine.graphics_wrapper_->CreateTextureBinding(ci);
		}
		catch(std::runtime_error &e) {
			G_NOTIFY("Unable to load level cubemap" + sub_path + ".\n");
		}
	}
}

void CubemapSystem::Reserve(int n) {
	components.reserve(n);
}

CubemapComponent *CubemapSystem::GetClosestCubemap(glm::vec3 point) {
	float closestLength = FLT_MAX;
	float length;
	size_t j = 0;
	if (components.size() > 0) {
		for (size_t i = 0; i < components.size(); i++) {
			glm::vec3 p2 = point - components[i].position;
			length = sqrt(p2.x*p2.x + p2.y*p2.y + p2.z*p2.z);
			if (length < closestLength) {
				closestLength = length;
				j = i;
			}
		}
		return &components[j];
	}
	return nullptr;
}

CubemapComponent *CubemapSystem::AddCubemap(glm::vec3 position) {
	components.push_back(CubemapComponent());
	components[components.size() - 1].position = position;
	return &components[components.size() - 1];
}