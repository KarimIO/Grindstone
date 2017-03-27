#include "SLight.h"
#include "../Core/Engine.h"

void SLight::AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius) {
	engine.entities[entityID].components[COMPONENT_LIGHT_POINT] = (unsigned int)pointLights.size();
	pointLights.push_back(CPointLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, lightRadius));
}

void SLight::AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerSpotAngle, float outerSpotAngle) {
	engine.entities[entityID].components[COMPONENT_LIGHT_SPOT] = (unsigned int)spotLights.size();
	spotLights.push_back(CSpotLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, lightRadius, innerSpotAngle, outerSpotAngle));
}

void SLight::AddDirectionalLight(unsigned int entityID) {
	engine.entities[entityID].components[COMPONENT_LIGHT_DIRECTIONAL] = (unsigned int)directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID));
}

void SLight::AddPointLight(unsigned int entityID) {
	engine.entities[entityID].components[COMPONENT_LIGHT_POINT] = (unsigned int)pointLights.size();
	pointLights.push_back(CPointLight(entityID));
}

void SLight::AddSpotLight(unsigned int entityID) {
	engine.entities[entityID].components[COMPONENT_LIGHT_SPOT] = (unsigned int)spotLights.size();
	spotLights.push_back(CSpotLight(entityID));
}

void SLight::AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius) {
	engine.entities[entityID].components[COMPONENT_LIGHT_DIRECTIONAL] = (unsigned int)directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID, lightColor, intensity, engine.settings.enableShadows && castShadow, sunRadius));
}

void SLight::SetPointers(GraphicsWrapper *gw, SModel *gc) {
	graphicsWrapper = gw;
	geometryCache = gc;
}

void SLight::DrawShadows() {

	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	//unsigned int i = iteration;
	//if (i < pointLights.size()) {
	for (size_t i = 0; i < pointLights.size(); i++) {
		CPointLight *light = &pointLights[i];
		if (light->castShadow) {
			unsigned int entityID = light->entityID;
			EBase *entity = &engine.entities[entityID];
			CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];
			glm::mat4 proj = glm::perspective(1.5708f, 1.0f, 0.1f, light->lightRadius);

			engine.graphicsWrapper->SetResolution(0, 0, 256, 256);
			light->fbo->WriteBind();
			graphicsWrapper->SetDepth(1);
			graphicsWrapper->SetCull(CULL_BACK);
			graphicsWrapper->SetBlending(false);
			for (unsigned int j = 0; j < 6; j++) {
				light->fbo->WriteBindFace(0, j);
				graphicsWrapper->Clear(CLEAR_ALL);
				glm::mat4 view = glm::lookAt(
					transform->GetPosition(),
					transform->GetPosition() + gCubeDirections[j].Target,
					gCubeDirections[j].Up
				);

				geometryCache->Draw(proj, view);
				engine.terrainSystem.Draw(proj, view, transform->position);
			}
			light->fbo->Unbind();
		}
	}

	//else if (i < spotLights.size() + pointLights.size()) {
	//	i -= pointLights.size();
	for (size_t i = 0; i < spotLights.size(); i++) {
		CSpotLight *light = &spotLights[i];
		if (light->castShadow) {
			unsigned int entityID = light->entityID;
			EBase *entity = &engine.entities[entityID];
			CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];
			glm::mat4 proj = glm::perspective(light->outerSpotAngle*2, 1.0f, 0.1f, light->lightRadius);
			glm::mat4 view = glm::lookAt(
				transform->GetPosition(),
				transform->GetPosition()+ transform->GetForward(),
				transform->GetUp()
			);

			light->projection = biasMatrix * proj * view * glm::mat4(1.0f);
			engine.graphicsWrapper->SetResolution(0, 0, 128, 128);
			light->fbo->WriteBind();
			graphicsWrapper->SetDepth(1);
			graphicsWrapper->SetCull(CULL_BACK);
			graphicsWrapper->SetBlending(false);
			graphicsWrapper->Clear(CLEAR_ALL);
			geometryCache->Draw(proj, view);
			engine.terrainSystem.Draw(proj, view, transform->position);
			light->fbo->Unbind();
		}
	}


	if (directionalLights.size() > 0) {
		unsigned int eID = directionalLights[0].entityID;
		EBase *entity = &engine.entities[eID];
		CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];
		float time = (float)engine.GetTimeCurrent();
		float ang = std::fmod(time / 4.0f, 360.0f);
		transform->position = glm::vec3(0, 2.0 + glm::sin(ang) * 32.0f, glm::cos(ang) * 32.0f);
		transform->angles = glm::vec3((ang+3.14159f), 0, 0);
	}

	for (size_t j = 0; j < directionalLights.size(); j++) {
		CDirectionalLight *light = &directionalLights[j];
		if (light->castShadow) {
			unsigned int entityID = light->entityID;
			EBase *entity = &engine.entities[entityID];
			CTransform *transform = &engine.transformSystem.components[entity->components[COMPONENT_TRANSFORM]];
			glm::mat4 proj = glm::ortho<float>(-64, 64, -64, 64, 0.1f, 64);
			glm::mat4 view = glm::lookAt(
				transform->GetPosition(),
				transform->GetPosition()+ transform->GetForward(),
				transform->GetUp()
			);

			light->projection = biasMatrix * proj * view * glm::mat4(1.0f);
			engine.graphicsWrapper->SetResolution(0, 0, 2048, 2048);
			light->fbo->WriteBind();
			graphicsWrapper->SetDepth(1);
			graphicsWrapper->SetCull(CULL_BACK);
			graphicsWrapper->SetBlending(false);
			graphicsWrapper->Clear(CLEAR_ALL);
			geometryCache->Draw(proj, view);
			engine.terrainSystem.Draw(proj, view, transform->position);
			light->fbo->Unbind();
		}
	}

	/*if (iteration >= spotLights.size() + pointLights.size())
		iteration++;
	else
		iteration = 0;*/
}
