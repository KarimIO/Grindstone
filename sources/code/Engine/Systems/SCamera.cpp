#include "SCamera.hpp"
#include "../Core/Engine.hpp"
#include "../Core/Utilities.hpp"

struct CamUBO {
	int texture = 0;
	float exposure;
} camUBO;

CCamera::CCamera() {
	aperture = 16;
	shutterSpeed = 1.0f / 200.0f;
	iso = 200.0f;
	camNear = 0.1f;
	camFar = 100.0f;
	fov = engine.settings.fov;
	aspectRatio = float(engine.settings.resolutionX) / engine.settings.resolutionY;

	/*std::string vsPath = "../shaders/overlay.glvs";
	std::string fsPath = "../shaders/post/ManualExposure.glfs";

	std::string vsContent, fsContent;

	if (!ReadFileIncludable(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	if (!ReadFileIncludable(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	shader = pfnCreateShader();
	shader->Initialize(2);
	if (!shader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!shader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", fsPath.c_str());
	if (!shader->Compile())
		fprintf(stderr, "Failed to compile program with: %s.\n", vsPath.c_str());

	shader->SetNumUniforms(2);
	shader->CreateUniform("tex");
	shader->CreateUniform("exposure");

	fsContent.clear();
	fsPath.clear();

	glm::vec2 res = glm::vec2(engine.settings.resolutionX, engine.settings.resolutionY);
	unsigned int resx = (unsigned int)res.x;
	unsigned int resy = (unsigned int)res.y;
#ifdef _WIN32 // Remove this ASAP
	const int GL_RGBA = 0x1908;
	const int GL_FLOAT = 0x1406;
	const int GL_RGBA32F = 0x8814;
#endif

	fbo = pfnCreateFramebuffer();
	fbo->Initialize(1);
	fbo->AddBuffer(GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned int)res.x, (unsigned int)res.y);
	fbo->Generate();*/
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

void CCamera::PostProcessing(Framebuffer *target) {
	/*target->ReadBind();
	target->BindTexture(0);
	target->Unbind();
	shader->Use();
	camUBO.exposure = glm::pow(2, CalculateExposure());

	shader->PassData(&camUBO);
	shader->SetInteger();
	shader->SetUniformFloat();

	fbo->WriteBind();
	engine.graphics_wrapper_->Clear(CLEAR_ALL);

	engine.vaoQuad->Bind();
	engine.graphics_wrapper_->DrawVertexArray(4);
	engine.vaoQuad->Unbind();
	fbo->Unbind();

	if (engine.graphics_wrapper_->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";*/
}

Framebuffer *CCamera::GetFramebuffer() {
	return fbo;
}

float CCamera::CalculateExposure(float middleVal) {
	float l_avg = (1000.0f / 65.0f) * aperture * aperture / (iso * shutterSpeed);
	return middleVal / l_avg;
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
