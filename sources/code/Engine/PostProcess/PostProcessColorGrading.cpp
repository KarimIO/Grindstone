#include "PostProcessColorGrading.hpp"
#include <string>
#include "../Core/Utilities.hpp"
#include "../Core/Engine.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "AssetManagers/TextureManager.hpp"
#include <GraphicsCommon/Texture.hpp>
#include <GraphicsCommon/Framebuffer.hpp>
#include <GraphicsCommon/GraphicsPipeline.hpp>

PostProcessColorGrading::PostProcessColorGrading(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *target, Grindstone::GraphicsAPI::Framebuffer **target_fbo) : BasePostProcess(pipeline), target_(target), target_fbo_(target_fbo) {
	reloadGraphics(w, h);
}

void PostProcessColorGrading::Process() {
	GRIND_PROFILE_FUNC();
	double dt = engine.getUpdateTimeDelta();

	gpipeline_->Bind();
	if (target_fbo_ && target_fbo_[0]) {
		target_fbo_[0]->Bind(true);
		// target_fbo_[0]->Clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
	}
	else {
		engine.getGraphicsWrapper()->bindDefaultFramebuffer(true);
		float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		engine.getGraphicsWrapper()->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);
	}
	target_->framebuffer->BindRead();
	target_->framebuffer->BindTextures(0);
	
	engine.getGraphicsWrapper()->bindTextureBinding(texture_binding_);

	engine.getGraphicsWrapper()->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
}

PostProcessColorGrading::~PostProcessColorGrading() {
}

void PostProcessColorGrading::resizeBuffers(unsigned int w, unsigned h)
{
}

void PostProcessColorGrading::reloadGraphics(unsigned int w, unsigned h) {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/grading.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
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
	stages[0].type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Tonemapping Fragment Shader missing.\n");
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo gradingGPCI;
	gradingGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	gradingGPCI.vertex_bindings = &vertex_layout;
	gradingGPCI.vertex_bindings_count = 1;
	gradingGPCI.width = (float)settings->resolution_x_;
	gradingGPCI.height = (float)settings->resolution_y_;
	gradingGPCI.scissorW = settings->resolution_x_;
	gradingGPCI.scissorH = settings->resolution_y_;
	gradingGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	gradingGPCI.shaderStageCreateInfos = stages;
	gradingGPCI.shaderStageCreateInfoCount = 2;

	Grindstone::GraphicsAPI::TextureSubBinding *grading_sub_binding_ = new Grindstone::GraphicsAPI::TextureSubBinding[2];
	grading_sub_binding_[0] = Grindstone::GraphicsAPI::TextureSubBinding("img", 0);
	grading_sub_binding_[1] = Grindstone::GraphicsAPI::TextureSubBinding("lut", 1);

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = grading_sub_binding_;
	tblci.bindingCount = 2;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	grading_tbl_ = graphics_wrapper->createTextureBindingLayout(tblci);

	gradingGPCI.textureBindings = &grading_tbl_;
	gradingGPCI.textureBindingCount = 1;
	gradingGPCI.uniformBufferBindings = nullptr;
	gradingGPCI.uniformBufferBindingCount = 0;
	gpipeline_ = graphics_wrapper->createGraphicsPipeline(gradingGPCI);
	delete grading_sub_binding_;

	// LUT
	Grindstone::GraphicsAPI::TextureOptions options;
	options.wrap_mode_u =	Grindstone::GraphicsAPI::TextureWrapMode::ClampToEdge;
	options.wrap_mode_v = Grindstone::GraphicsAPI::TextureWrapMode::ClampToEdge;
	options.min_filter = Grindstone::GraphicsAPI::TextureFilter::Linear;
	options.mag_filter = Grindstone::GraphicsAPI::TextureFilter::Linear;
	options.generate_mipmaps = false;

	auto texmgr = engine.getTextureManager();
	TextureHandler handle = texmgr->loadTexture("../engineassets/materials/postprocessing/lut.png", options);
	if (handle == size_t(-1)) {
		texture_ = nullptr;
		texture_binding_ = nullptr;
	}
	else {
		Grindstone::GraphicsAPI::Texture *texture = texmgr->getTexture(handle);
		texture_ = texture;
		Grindstone::GraphicsAPI::SingleTextureBind stb;
		stb.texture = texture_;
		stb.address = 1;

		Grindstone::GraphicsAPI::TextureBindingCreateInfo ci;
		ci.textures = &stb;
		ci.layout = grading_tbl_;
		ci.textureCount = 1;
		texture_binding_ = graphics_wrapper->createTextureBinding(ci);
	}
}

void PostProcessColorGrading::destroyGraphics() {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();

	graphics_wrapper->deleteTextureBinding(texture_binding_);
	graphics_wrapper->deleteTextureBindingLayout(grading_tbl_);
	graphics_wrapper->deleteGraphicsPipeline(gpipeline_);
}
