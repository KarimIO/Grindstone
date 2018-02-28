#include "SCubemap.hpp"
#include <iostream>

#include "../Core/Engine.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

/*
materialManager.resetDraws();
geometry_system.Cull(cam);

pv = cam->GetProjection() * cam->GetView();
ubo->UpdateUniformBuffer(&pv);
ubo->Bind();
ubo2->Bind();

Render();
*/

void WriteImage(std::string path, unsigned int w, unsigned int h, unsigned int c, unsigned char *p) {
	stbi_write_png(path.c_str(), w, h, c, p, w * c);
}

void CubemapSystem::CaptureCubemaps(double) {
	std::cout << "Capture Cubemaps" << "\n";

	glm::mat4 proj = glm::perspective(1.5708f, 1.0f, 1.0f, 1000.0f);
	glm::mat4 view;

	writing = true;
	for (size_t i = 0; i < components.size(); i++) {

		std::string path = "../assets/cubemaps/level" + std::to_string(i);
		for (size_t j = 0; j < 6; j++) {
			view = glm::lookAt(components[i].position, components[i].position + gCubeDirections[j].Target, gCubeDirections[j].Up);
			view = proj * view;
			engine.ubo->UpdateUniformBuffer(&view);
			engine.ubo->Bind();

			engine.ubo2->Bind();

			engine.Render();
			engine.graphics_wrapper_->BindDefaultFramebuffer(true);
			unsigned char *data = engine.gbuffer_images_->RenderScreen(0);
			WriteImage((path + "_" + std::to_string(j) + ".png").c_str(), 1366, 768, 3, data);
		}
	}

	LoadCubemaps();

	writing = false;
}

void CubemapSystem::LoadCubemaps() {
	/*writing = false;
	for (size_t i = 0; i < components.size(); i++) {
		components[i].cubemap = engine.textureManager.LoadCubemap("../assets/cubemaps/level" + std::to_string(i), ".png", COLOR_SRGB);
	}*/
}

void CubemapSystem::Reserve(int n) {
	components.reserve(n);
}

CubemapComponent *CubemapSystem::GetClosestCubemap(glm::vec3 point) {
	if (writing)
		return NULL;

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