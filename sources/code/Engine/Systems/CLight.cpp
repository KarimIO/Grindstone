#include "CLight.h"
#include "../Core/GraphicsDLLPointer.h"
#include "Core/EBase.h"
#include "Core/Engine.h"

CPointLight::CPointLight(unsigned int entityID) {
	this->entityID = entityID;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = engine.pointLightUBB;
	lightUBO = engine.graphicsWrapper->CreateUniformBuffer(lightuboci);
}

CPointLight::CPointLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) {
	lightUBOBuffer.attenuationRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = engine.pointLightUBB;
	lightUBO = engine.graphicsWrapper->CreateUniformBuffer(lightuboci);

	castShadow = cast;

#if 0
	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthCubeBuffer(256, 256);
		fbo->Generate();
	}
#endif
}

void CPointLight::SetShadow(bool state) {
	castShadow = state;
#if 0
	if (state) {
		fbo = pfnCreateFramebuffer();
		fbo->Initialize(0);
		fbo->AddDepthCubeBuffer(128, 128);
		fbo->Generate();
	}
#endif
}

void CPointLight::Bind() {
	EBase *entity = &engine.entities[entityID];
	unsigned int transID = entity->components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->position;
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

CSpotLight::CSpotLight(unsigned int entityID) {
	this->entityID = entityID;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = engine.spotLightUBB;
	lightUBO = engine.graphicsWrapper->CreateUniformBuffer(lightuboci);
}

CSpotLight::CSpotLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa) {
	lightUBOBuffer.innerAngle = ia * 3.14159265359f / 360.0f;
	lightUBOBuffer.outerAngle = oa * 3.14159265359f / 360.0f;
	lightUBOBuffer.attenuationRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;

	castShadow = cast;

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = engine.spotLightUBB;
	lightUBO = engine.graphicsWrapper->CreateUniformBuffer(lightuboci);

#if 0
	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthBuffer(256, 256);
		fbo->Generate();
	}
#endif
}

void CSpotLight::SetShadow(bool state) {
	castShadow = state;
#if 0
	if (state) {
		fbo = pfnCreateFramebuffer();
		fbo->Initialize(0);
		fbo->AddDepthBuffer(128, 128);
		fbo->Generate();
	}
#endif
}

void CSpotLight::Bind() {
	EBase *entity = &engine.entities[entityID];
	unsigned int transID = entity->components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->GetPosition();
	lightUBOBuffer.direction = trans->GetForward();
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}

CDirectionalLight::CDirectionalLight(unsigned int entityID) {
	this->entityID = entityID;
}

CDirectionalLight::CDirectionalLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) {
	lightUBOBuffer.sourceRadius = radius;
	lightUBOBuffer.color = color;
	lightUBOBuffer.power = strength;

	castShadow = cast;

#if 0
	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthBuffer(128, 128);
		fbo->Generate();
	}
#endif
}

void CDirectionalLight::SetShadow(bool state) {
	castShadow = state;
#if 0
	if (state) {
		fbo = pfnCreateFramebuffer();
		fbo->Initialize(0);
		fbo->AddDepthBuffer(2048, 2048);
		fbo->Generate();
	}
#endif
}

void CDirectionalLight::Bind() {
	EBase *entity = &engine.entities[entityID];
	unsigned int transID = entity->components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transID];
	lightUBOBuffer.position = trans->position;
	lightUBO->UpdateUniformBuffer(&lightUBOBuffer);
	lightUBO->Bind();
}