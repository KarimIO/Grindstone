#include "SLight.h"
#include "Engine.h"

void SLight::AddPointLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius) {
	engine.entities[entityID].components[COMPONENT_LIGHT_POINT] = pointLights.size();
	pointLights.push_back(CPointLight(entityID, lightColor, intensity, castShadow, lightRadius));
}

void SLight::AddSpotLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float lightRadius, float innerSpotAngle, float outerSpotAngle) {
	engine.entities[entityID].components[COMPONENT_LIGHT_SPOT] = spotLights.size();
	spotLights.push_back(CSpotLight(entityID, lightColor, intensity, castShadow, lightRadius, innerSpotAngle, outerSpotAngle));
}

void SLight::AddDirectionalLight(unsigned int entityID, glm::vec3 lightColor, float intensity, bool castShadow, float sunRadius) {
	engine.entities[entityID].components[COMPONENT_LIGHT_DIRECTIONAL] = directionalLights.size();
	directionalLights.push_back(CDirectionalLight(entityID, lightColor, intensity, castShadow, sunRadius));
}

void SLight::SetPointers(GraphicsWrapper *gw, SModel *gc) {
	graphicsWrapper = gw;
	geometryCache = gc;
	iteration = 0;
}

void SLight::DrawShadows() {

	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	unsigned int i = iteration;
	if (i < pointLights.size()) {
	//for (size_t i = 0; i < 0; i++) {
		CPointLight *light = &pointLights[i];
		if (light->castShadow) {
			unsigned int entityID = light->entityID;
			EBase *entity = &engine.entities[entityID];
			glm::mat4 proj = glm::perspective(1.5708f, 1.0f, 0.1f, light->lightRadius);
			glm::mat4 view = glm::lookAt(
				entity->GetPosition(),
				entity->GetPosition() + entity->GetForward(),
				entity->GetUp()
			);

			light->projection = biasMatrix * proj * view * glm::mat4(1.0f);
			engine.graphicsWrapper->SetResolution(0, 0, 256, 256);
			light->fbo->WriteBind();
			graphicsWrapper->SetDepth(1);
			graphicsWrapper->SetCull(CULL_FRONT);
			graphicsWrapper->SetBlending(false);
			graphicsWrapper->Clear(CLEAR_ALL);
			geometryCache->Draw(proj, view);
			light->fbo->Unbind();
		}
	}

	else if (i < spotLights.size() + pointLights.size()) {
		i -= pointLights.size();
	//for (size_t i = 0; i < spotLights.size(); i++) {
		CSpotLight *light = &spotLights[i];
		if (light->castShadow) {
			unsigned int entityID = light->entityID;
			EBase *entity = &engine.entities[entityID];
			glm::mat4 proj = glm::perspective(light->outerSpotAngle*2, 1.0f, 0.1f, light->lightRadius);
			glm::mat4 view = glm::lookAt(
				entity->GetPosition(),
				entity->GetPosition()+entity->GetForward(),
				entity->GetUp()
			);

			light->projection = biasMatrix * proj * view * glm::mat4(1.0f);
			engine.graphicsWrapper->SetResolution(0, 0, 256, 256);
			light->fbo->WriteBind();
			graphicsWrapper->SetDepth(1);
			graphicsWrapper->SetCull(CULL_FRONT);
			graphicsWrapper->SetBlending(false);
			graphicsWrapper->Clear(CLEAR_ALL);
			geometryCache->Draw(proj, view);
			light->fbo->Unbind();
		}
	}

	for (size_t j = 0; j < directionalLights.size(); j++) {
		CDirectionalLight *light = &directionalLights[j];
		if (light->castShadow) {
			unsigned int entityID = light->entityID;
			EBase *entity = &engine.entities[entityID];
			glm::mat4 proj = glm::ortho<float>(-64, 64, -64, 64, -8, 64);
			glm::mat4 view = glm::lookAt(
				entity->GetPosition(),
				glm::vec3(0), //entity->GetPosition()+entity->GetForward(),
				entity->GetUp()
			);

			light->projection = biasMatrix * proj * view * glm::mat4(1.0f);
			engine.graphicsWrapper->SetResolution(0, 0, 1024, 1024);
			light->fbo->WriteBind();
			graphicsWrapper->SetDepth(1);
			graphicsWrapper->SetCull(CULL_FRONT);
			graphicsWrapper->SetBlending(false);
			graphicsWrapper->Clear(CLEAR_ALL);
			geometryCache->Draw(proj, view);
			light->fbo->Unbind();
		}
	}

	if (iteration >= spotLights.size() + pointLights.size())
		iteration++;
	else
		iteration = 0;
}
