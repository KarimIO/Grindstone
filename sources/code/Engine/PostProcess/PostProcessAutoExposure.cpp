#include "../Core/Engine.hpp"
#include <GraphicsWrapper.hpp>
#include "Core/Utilities.hpp"
#include "PostProcessAutoExposure.hpp"

PostProcessAutoExposure::PostProcessAutoExposure(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target) : BasePostProcess(pipeline), source_(source), target_(target), lum_buffer_(nullptr), lum_framebuffer_(nullptr), gpipeline_(nullptr){
	auto gw = engine.getGraphicsWrapper();
	tonemap_sub_binding_ = new Grindstone::GraphicsAPI::TextureSubBinding("lighting", 4);

	reloadGraphics(w, h);
}

PostProcessAutoExposure::~PostProcessAutoExposure() {
	destroyGraphics();
	delete tonemap_sub_binding_;
}

void PostProcessAutoExposure::resizeBuffers(unsigned int w, unsigned h) {
	auto gw = engine.getGraphicsWrapper();
	auto gl = engine.getSettings()->graphics_language_;

	if (lum_buffer_) {
		gw->DeleteRenderTarget(lum_buffer_);
		lum_buffer_ = nullptr;
	}
	if (lum_framebuffer_) {
		gw->DeleteFramebuffer(lum_framebuffer_);
		lum_framebuffer_ = nullptr;
	}
	if (gpipeline_) {
		gw->DeleteGraphicsPipeline(gpipeline_);
		gpipeline_ = nullptr;
	}

	Grindstone::GraphicsAPI::RenderTargetCreateInfo lum_buffer_ci(Grindstone::GraphicsAPI::ColorFormat::R8, 1024, 1024);
	lum_buffer_ = gw->CreateRenderTarget(&lum_buffer_ci, 1);

	Grindstone::GraphicsAPI::FramebufferCreateInfo lum_framebuffer_ci;
	lum_framebuffer_ci.render_target_lists = &lum_buffer_;
	lum_framebuffer_ci.num_render_target_lists = 1;
	lum_framebuffer_ci.depth_target = nullptr;
	lum_framebuffer_ci.render_pass = nullptr;
	lum_framebuffer_ = gw->CreateFramebuffer(lum_framebuffer_ci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (gl == GraphicsLanguage::OpenGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/luminance.glsl";
	}
	else if (gl == GraphicsLanguage::DirectX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/luminance.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/luminance.spv";
	}

	std::vector<char> vfile;
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("Luminance Vertex Shader missing.\n");
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Luminance Fragment Shader missing.\n");
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo luminanceGPCI;
	luminanceGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	luminanceGPCI.bindings = &vbd;
	luminanceGPCI.bindingsCount = 1;
	luminanceGPCI.attributes = &vad;
	luminanceGPCI.attributesCount = 1;
	luminanceGPCI.width = (float)1024;
	luminanceGPCI.height = (float)1024;
	luminanceGPCI.scissorW = 1024;
	luminanceGPCI.scissorH = 1024;
	luminanceGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	luminanceGPCI.shaderStageCreateInfos = stages;
	luminanceGPCI.shaderStageCreateInfoCount = 2;

	luminanceGPCI.textureBindings = &tonemap_tbl_;
	luminanceGPCI.textureBindingCount = 1;
	luminanceGPCI.uniformBufferBindings = nullptr;
	luminanceGPCI.uniformBufferBindingCount = 0;
	gpipeline_ = gw->CreateGraphicsPipeline(luminanceGPCI);
}

void PostProcessAutoExposure::reloadGraphics(unsigned int w, unsigned h) {
	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = tonemap_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	tonemap_tbl_ = engine.getGraphicsWrapper()->CreateTextureBindingLayout(tblci);
	resizeBuffers(w, h);
}

void PostProcessAutoExposure::destroyGraphics() {
	auto gw = engine.getGraphicsWrapper();
	if (lum_buffer_) {
		gw->DeleteRenderTarget(lum_buffer_);
		lum_buffer_ = nullptr;
	}
	if (lum_framebuffer_) {
		gw->DeleteFramebuffer(lum_framebuffer_);
		lum_framebuffer_ = nullptr;
	}
	if (gpipeline_) {
		gw->DeleteGraphicsPipeline(gpipeline_);
		gpipeline_ = nullptr;
	}
	if (tonemap_tbl_) {
		gw->DeleteTextureBindingLayout(tonemap_tbl_);
		tonemap_tbl_ = nullptr;
	}
}

void PostProcessAutoExposure::Process() {
	GRIND_PROFILE_FUNC();
    gpipeline_->Bind();
	lum_framebuffer_->BindWrite(false);
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(4);
    engine.getGraphicsWrapper()->DrawImmediateVertices(0, 6);
}

float PostProcessAutoExposure::GetExposure() {
	return source_->render_targets[0]->getAverageValue(0);
}