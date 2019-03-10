#include "PostProcessColorGrading.hpp"
#include <string>
#include "../Core/Utilities.hpp"
#include "../Core/Engine.hpp"
#include <GraphicsWrapper.hpp>
#include "AssetManagers/TextureManager.hpp"
#include "Texture.hpp"

PostProcessColorGrading::PostProcessColorGrading(PostPipeline *pipeline, RenderTargetContainer *target) : BasePostProcess(pipeline), target_(target) {
	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (settings->graphics_language_ == GRAPHICS_OPENGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/grading.glsl";
	}
	else if (settings->graphics_language_ == GRAPHICS_DIRECTX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/grading.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/grading.spv";
	}

	std::vector<char> vfile;
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("Tonemapping Vertex Shader missing.\n");
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Tonemapping Fragment Shader missing.\n");
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = SHADER_FRAGMENT;

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	GraphicsPipelineCreateInfo gradingGPCI;
	gradingGPCI.cullMode = CULL_BACK;
	gradingGPCI.bindings = &vbd;
	gradingGPCI.bindingsCount = 1;
	gradingGPCI.attributes = &vad;
	gradingGPCI.attributesCount = 1;
	gradingGPCI.width = (float)settings->resolution_x_;
	gradingGPCI.height = (float)settings->resolution_y_;
	gradingGPCI.scissorW = settings->resolution_x_;
	gradingGPCI.scissorH = settings->resolution_y_;
	gradingGPCI.primitiveType = PRIM_TRIANGLES;
	gradingGPCI.shaderStageCreateInfos = stages;
	gradingGPCI.shaderStageCreateInfoCount = 2;

	TextureSubBinding *grading_sub_binding_ = new TextureSubBinding[2];
	grading_sub_binding_[0] = TextureSubBinding("img", 0);
	grading_sub_binding_[1] = TextureSubBinding("lut", 1);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = grading_sub_binding_;
	tblci.bindingCount = 2;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *grading_tbl_ = graphics_wrapper->CreateTextureBindingLayout(tblci);

	gradingGPCI.textureBindings = &grading_tbl_;
	gradingGPCI.textureBindingCount = 1;
	gradingGPCI.uniformBufferBindings = nullptr;
	gradingGPCI.uniformBufferBindingCount = 0;
	gpipeline_ = graphics_wrapper->CreateGraphicsPipeline(gradingGPCI);

	// LUT
	TextureOptions options;
	options.wrap_mode_u = TEXWRAP_CLAMP_TO_EDGE;
	options.wrap_mode_v = TEXWRAP_CLAMP_TO_EDGE;
	options.min_filter = TEXFILTER_LINEAR;
	options.mag_filter = TEXFILTER_LINEAR;
	options.generate_mipmaps = false;

	auto texmgr = engine.getTextureManager();
	std::string path = "lut.png";
	std::string fullpath = std::string("../assets/materials/") + path;
	TextureHandler handle = texmgr->loadTexture(fullpath, options);
	if (handle == size_t(-1)) {
		texture_ = nullptr;
		texture_binding_ = nullptr;
	}
	else {
		Texture *texture = texmgr->getTexture(handle);
		texture_ = texture;
		SingleTextureBind stb;
		stb.texture = texture_;
		stb.address = 1;

		TextureBindingCreateInfo ci;
		ci.textures = &stb;
		ci.layout = grading_tbl_;
		ci.textureCount = 1;
		texture_binding_ = graphics_wrapper->CreateTextureBinding(ci);
	}

}

void PostProcessColorGrading::Process() {
	double dt = engine.getUpdateTimeDelta();

	gpipeline_->Bind();
	engine.getGraphicsWrapper()->BindDefaultFramebuffer(true);
	target_->framebuffer->BindRead();
	target_->framebuffer->BindTextures(0);
	
	engine.getGraphicsWrapper()->BindTextureBinding(texture_binding_);

	engine.getGraphicsWrapper()->Clear(CLEAR_BOTH);
	engine.getGraphicsWrapper()->DrawImmediateVertices(0, 6);
}

PostProcessColorGrading::~PostProcessColorGrading() {
	/*shader->Cleanup();
	//fbo->Cleanup();
	pfnDeleteGraphicsPointer(shader);
	pfnDeleteGraphicsPointer(fbo);*/
}
