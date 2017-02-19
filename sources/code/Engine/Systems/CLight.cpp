#include "CLight.h"
#include "../Core/GraphicsDLLPointer.h"

CBaseLight::CBaseLight(unsigned int entID, glm::vec3 color, float strength, bool cast) {
	lightColor = color;
	intensity = strength;
	castShadow = cast;
	entityID = entID;
	
	if (cast)
		fbo = pfnCreateFramebuffer();
}

CPointLight::CPointLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) : CBaseLight(entID, color, strength, cast) {
	lightRadius = radius;

	if (false&&cast) {
		fbo->Initialize(0);
		fbo->AddDepthCubeBuffer(256, 256);
		fbo->Generate();
	}
}

CSpotLight::CSpotLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius, float ia, float oa) : CBaseLight(entID, color, strength, cast) {
	innerSpotAngle = ia * 3.14159265359f / 360.0f;
	outerSpotAngle = oa * 3.14159265359f / 360.0f;
	lightRadius = radius;

	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthBuffer(256, 256);
		fbo->Generate();
	}
}

CDirectionalLight::CDirectionalLight(unsigned int entID, glm::vec3 color, float strength, bool cast, float radius) : CBaseLight(entID, color, strength, cast) {
	sunRadius = radius;

	if (cast) {
		fbo->Initialize(0);
		fbo->AddDepthBuffer(1024, 1024);
		fbo->Generate();
	}
}
