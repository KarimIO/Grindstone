#include "SCubemap.h"
#include <iostream>

#include "../Core/TextureManager.h"

#include "../Core/Engine.h"

#include <Windows.h>

void CubemapSystem::CaptureCubemaps(double) {
	std::cout << "Capture Cubemaps" << "\n";
	writing = true;
	for (size_t i = 0; i < components.size(); i++) {

		glm::mat4 Proj = glm::perspective(1.5708f, 1.0f, 1.0f, 1000.0f);
		glm::mat4 View;

		engine.graphicsWrapper->SetResolution(0, 0, 512, 512);
		std::string path = "";
		for (size_t j = 0; j < 6; j++) {
			path = "../cubemaps/level" + std::to_string(i) + (gCubeDirections[j].name) + ".png";
			View = glm::lookAt(components[i].position, components[i].position + gCubeDirections[j].Target, gCubeDirections[j].Up);

			engine.Render(Proj, View, false);
			engine.graphicsWrapper->SwapBuffer();
			unsigned char *data = engine.graphicsWrapper->ReadScreen(512, 512);
			WriteTexture(path.c_str(), 512, 512, 3, data);
			free(data);
		}
		engine.graphicsWrapper->SetResolution(0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
	}

	LoadCubemaps();
	writing = false;
	engine.graphicsWrapper->SetResolution(0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
}

void CubemapSystem::LoadCubemaps() {
	writing = false;
	for (size_t i = 0; i < components.size(); i++) {
		components[i].cubemap = LoadCubemap("../cubemaps/level" + std::to_string(i), ".png", COLOR_SRGB);
	}
}

CubemapComponent *CubemapSystem::GetClosestCubemap(glm::vec3 point) {
	if (writing)
		return NULL;

	float closestLength = FLT_MAX;
	float length;
	int j = 0;
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