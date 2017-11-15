#include "ColorGradingPost.hpp"
#include <string>
#include "../Core/Utilities.hpp"
#include "../Core/Engine.hpp"

ColorGradingPost::ColorGradingPost() {
	/*std::string vsPath = "../shaders/overlay.glvs";
	std::string fsPath = "../shaders/post/ColorGrading.glfs";

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

	shader->SetNumUniforms(1);
	shader->CreateUniform("tex");

	fsContent.clear();
	fsPath.clear();*/
}

ColorGradingPost::ColorGradingPost(const ColorGradingPost &cp) {

}

ColorGradingPost::ColorGradingPost(ColorGradingPost &&mv) {

}

Framebuffer *ColorGradingPost::Process(Framebuffer *target) {
	/*target->ReadBind();
	target->BindTexture(0);
	target->Unbind();
	shader->Use();
	int texture = 0;
	shader->PassData(&texture);
	shader->SetInteger();

	//fbo->WriteBind();
	engine.graphics_wrapper_->Clear(CLEAR_ALL);

	engine.vaoQuad->Bind();
	engine.graphics_wrapper_->DrawVertexArray(4);
	engine.vaoQuad->Unbind();
	//fbo->Unbind();

	if (engine.graphics_wrapper_->CheckForErrors())
		std::cout << "Error was at " << __LINE__ << ", in " << __FILE__ << " \n";
		
	return fbo;*/
}

ColorGradingPost::~ColorGradingPost() {
	/*shader->Cleanup();
	//fbo->Cleanup();
	pfnDeleteGraphicsPointer(shader);
	pfnDeleteGraphicsPointer(fbo);*/
}
