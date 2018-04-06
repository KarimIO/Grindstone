#include "SCamera.hpp"
#include "../Core/Engine.hpp"
#include "../Core/Utilities.hpp"
#include "../PostProcess/PostProcessTonemap.hpp"
#include "../PostProcess/PostProcessSSAO.hpp"
#include "../PostProcess/PostProcessIBL.hpp"
#include "../PostProcess/PostProcessAutoExposure.hpp"

CCamera::CCamera() {
	aperture = 16;
	shutterSpeed = 1.0f / 200.0f;
	iso = 200.0f;
	camNear = 0.1f;
	camFar = 100.0f;
	fov = engine.settings.fov;
	aspectRatio = float(engine.settings.resolutionX) / engine.settings.resolutionY;


	RenderTargetContainer *rt_hdr = &engine.rt_hdr_;

	if (engine.settings.use_ssao) {
		BasePostProcess *pp_ssao = new PostProcessSSAO(&engine.rt_gbuffer_);
		post_pipeline_.AddPostProcess(pp_ssao); 
	}

	if (engine.settings.enableReflections) {
		BasePostProcess *pp_ibl = new PostProcessIBL(&engine.rt_gbuffer_, rt_hdr);
		post_pipeline_.AddPostProcess(pp_ibl);
	}

	PostProcessAutoExposure *pp_auto = new PostProcessAutoExposure(rt_hdr, nullptr);
	post_pipeline_.AddPostProcess(pp_auto);

	PostProcessTonemap *pp_tonemap = new PostProcessTonemap(rt_hdr, nullptr, pp_auto);
	post_pipeline_.AddPostProcess(pp_tonemap);
}

void CCamera::SetSize(float X, float Y, float Width, float Height) {
	x = X;
	y = Y;
	width = Width;
	height = Height;
}

void CCamera::SetAspectRatio(float ratio) {
	aspectRatio = ratio;
}

void CCamera::SetLookAt(glm::vec3 pos) {
}

void CCamera::SetLookAt(float x, float y, float z) {
}

void CCamera::SetProjection(bool perspective) {
}

void CCamera::SetNear(float Near) {
	camNear = Near;
}

void CCamera::SetFar(float Far) {
	camFar = Far;
}

void CCamera::SetShutterSpeed(float _speed) {
	shutterSpeed = _speed;
}

void CCamera::SetApertureSize(float _aperture) {
	aperture = _aperture;
}

void CCamera::SetISO(float _iso) {
	iso = _iso;
}

void CCamera::SetFOV(float _fov) {
	fov = _fov;
}

void CCamera::PostProcessing() {
	//pp_tonemap->Process();
	post_pipeline_.Process();
}

glm::mat4 CCamera::GetProjection() {
	glm::mat4 projection = glm::perspective(fov, aspectRatio, camNear, camFar);
	if (engine.settings.graphicsLanguage == GRAPHICS_VULKAN)
		projection[1][1] *= -1;
	
	const glm::mat4 scale = glm::mat4(1.0f,0,0,0,
									0,1.0f,0,0,
									0,0,0.5f,0,
									0,0,0.25f,1.0f);

	if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX)
		projection = scale * projection;

	return projection;
}

glm::mat4 CCamera::GetView() {
	CTransform *transform = &engine.transformSystem.components[engine.entities[entityID].components_[COMPONENT_TRANSFORM]];
	glm::mat4 view = glm::lookAt(
		transform->GetPosition(),
		transform->GetPosition() + transform->GetForward(),
		transform->GetUp()
	);

	return view;
}

float CCamera::GetNear() {
	return camNear;
}

float CCamera::GetFar()
{
	return camFar;
}

float CCamera::GetAspectRatio()
{
	return aspectRatio;
}

float CCamera::GetFOV() {
	return fov;
}

void SCamera::AddComponent(unsigned int entID, unsigned int & target) {
	components.push_back(CCamera());
	components[components.size() - 1].entityID = entID;
	target = (unsigned int)(components.size() - 1);
}
