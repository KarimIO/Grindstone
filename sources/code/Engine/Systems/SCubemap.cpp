#include "SCubemap.hpp"
#include <iostream>

#include "../Core/Engine.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

void WriteImage(std::string path, unsigned int w, unsigned int h, unsigned int c, unsigned char *p) {
	stbi_write_png(path.c_str(), w, h, c, p, w * c);
}

void CubemapSystem::CaptureCubemaps(double) {
	std::cout << "Capture Cubemaps" << "\n";

	glm::mat4 proj = glm::perspective(1.5708f, 1.0f, 0.1f, 1000.0f);
	glm::mat4 view;

	int res = 512;


	if (!engine.settings.debugNoLighting && engine.settings.enableShadows)
		engine.lightSystem.DrawShadows();

	bool shouldUseShadows = engine.settings.enableShadows;
	engine.settings.enableShadows = false;

	int num_passes = 3;

	for (int k = 0; k < num_passes; ++k) {
		for (size_t i = 0; i < components.size(); ++i) {

			std::string path = "../assets/cubemaps/" + engine.level_file_name_ + "_" + std::to_string(i);
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
				unsigned char *data = engine.gbuffer_images_->RenderScreen(0, 512, 512);
				WriteImage((path + "_" + gCubeDirections[j].name + ".png").c_str(), 512, 512, 3, data);
			}
		}
		engine.settings.enableShadows = true;
		LoadCubemaps();
	}

	engine.settings.enableShadows = shouldUseShadows;
}

void CubemapSystem::LoadCubemaps() {
	G_NOTIFY("Loading Cubemaps.\n");
	std::string path = "../assets/cubemaps/" + engine.level_file_name_ + "_";
	for (size_t i = 0; i < components.size(); i++) {
		std::string sub_path = path + std::to_string(i) + "_.png";
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