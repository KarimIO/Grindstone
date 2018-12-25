#include "CubemapSystem.hpp"
#include <iostream>

#include "../Core/Engine.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_DXT_IMPLEMENTATION
#include <stb/stb_dxt.h>

#include <fstream>

#include <cstring>

#include "../Utilities/Logger.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include "TransformSystem.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include <GraphicsWrapper.hpp>

/*void ExtractBlock(const unsigned char* inPtr, unsigned int width, unsigned char* colorBlock) {
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


	if (engine.settings.enableShadows)
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

				MainUBO ubo;
				ubo.pv = proj * view;
				ubo.eye_pos = components[i].position;
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
*/

CubemapComponent::CubemapComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_CUBEMAP, object_handle, id), capture_method_(CubemapComponent::CaptureMethod::CAPTURE_BAKE), near_(0.1f), far_(100.0f), resolution_(128) {}

CubemapSystem::CubemapSystem() : System(COMPONENT_CUBEMAP) {}

CubemapSubSystem::CubemapSubSystem(Space *space) : SubSystem(COMPONENT_CUBEMAP, space) {
	cube_binding_ = TextureSubBinding("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &cube_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	texture_binding_layout_ = engine.getGraphicsWrapper()->CreateTextureBindingLayout(tblci);
}

ComponentHandle CubemapSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	if (params.HasMember("type")) {
		std::string type = params["type"].GetString();

		if (type == "baked") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_BAKE;
		}
		else if (type == "realtime") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_REALTIME;
		}
		else if (type == "custom") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_CUSTOM;

			if (params.HasMember("path")) {
				std::string path = std::string("../assets/") + params["path"].GetString();

				// Load File
				TextureHandler handle = engine.getTextureManager()->loadCubemap(path);
				Texture *texture = engine.getTextureManager()->getTexture(handle);
				component.cubemap_ = texture;
				component.cubemap_binding_;

				SingleTextureBind stb;
				stb.texture = component.cubemap_;
				stb.address = 4;

				TextureBindingCreateInfo ci;
				ci.textures = &stb;
				ci.layout = texture_binding_layout_;
				ci.textureCount = 1;
				component.cubemap_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
			}
			else {
				LOG_WARN("No path given.");
			}
		}
		else {
			LOG_WARN("Invalid type.");
		}
	}

	if (component.capture_method_ == CubemapComponent::CaptureMethod::CAPTURE_BAKE ||
		component.capture_method_ == CubemapComponent::CaptureMethod::CAPTURE_REALTIME) {
		if (params.HasMember("resolution")) {
			component.resolution_ = params["resolution"].GetUint();
		}

		if (params.HasMember("far")) {
			component.far_ = params["far"].GetFloat();
		}

		if (params.HasMember("near")) {
			component.near_ = params["far"].GetFloat();
		}
	}

	return component_handle;
}


void CubemapSystem::update(double dt) {
	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
			}
		}
	}
}

CubemapComponent & CubemapSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t CubemapSubSystem::getNumComponents() {
	return components_.size();
}

void CubemapSubSystem::removeComponent(ComponentHandle handle) {
}

void CubemapSubSystem::captureCubemaps(double) {
}

void CubemapSubSystem::loadCubemaps() {
}

CubemapComponent * CubemapSubSystem::getClosestCubemap(glm::vec3 eye) {
	float dist_max = INFINITY;
	CubemapComponent *max = nullptr;

	for (auto &component : components_) {
		GameObjectHandle game_object_id = component.game_object_handle_;
		ComponentHandle transform_id = space_->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
		TransformSubSystem *transform = (TransformSubSystem *)(space_->getSubsystem(COMPONENT_TRANSFORM));

		glm::vec3 c = transform->getPosition(transform_id);
		float dist = glm::distance(eye, c);
		if (dist < dist_max) {
			max = &component;
			dist_max = dist;
		}
	}

	return max;
}

CubemapSubSystem::~CubemapSubSystem() {
}
