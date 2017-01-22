#include "CLight.h"
#include "GraphicsDLLPointer.h"

CBaseLight::CBaseLight(unsigned int entID, glm::vec3 color, float strength, bool cast) {
	lightColor = color;
	intensity = strength;
	castShadow = cast;
	entityID = entID;
	fbo = pfnCreateFramebuffer();
}

CPointLight::CPointLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) : CBaseLight(entID, color, strength, cast) {
	lightRadius = radius;

	//fbo->Initialize(0);
	//fbo->AddDepthCubeBuffer(256, 256);
	//fbo->Generate();
}

CSpotLight::CSpotLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa) : CPointLight(entID, color, strength, cast, radius) {
	innerSpotAngle = ia * 3.14159265359f / 360.0f;
	outerSpotAngle = oa * 3.14159265359f / 360.0f;

	fbo->Initialize(0);
	fbo->AddDepthBuffer(256, 256);
	fbo->Generate();
}

CDirectionalLight::CDirectionalLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) : CBaseLight(entID, color, strength, cast) {
	sunRadius = radius;

	fbo->Initialize(0);
	fbo->AddDepthBuffer(1024, 1024);
	fbo->Generate();
}
