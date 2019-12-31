#include "../Core/Engine.hpp"
#include <GraphicsWrapper.hpp>
#include "Core/Utilities.hpp"
#include "PostProcessBloom.hpp"
#include "PostProcessAutoExposure.hpp"

PostProcessBloom::PostProcessBloom(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure) : BasePostProcess(pipeline), source_(source), target_(target), auto_exposure_(auto_exposure) {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/bloom.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/bloom.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/bloom.spv";
	}

	std::vector<char> vfile;
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("Bloom Vertex Shader missing.\n");
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Bloom Fragment Shader missing.\n");
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

	Grindstone::GraphicsAPI::TextureSubBinding *tonemap_sub_binding_ = new Grindstone::GraphicsAPI::TextureSubBinding("lighting", 4);

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = tonemap_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	Grindstone::GraphicsAPI::TextureBindingLayout *tonemap_tbl_ = graphics_wrapper->CreateTextureBindingLayout(tblci);


	luminanceGPCI.textureBindings = &tonemap_tbl_;
	luminanceGPCI.textureBindingCount = 1;
	luminanceGPCI.uniformBufferBindings = nullptr;
	luminanceGPCI.uniformBufferBindingCount = 0;
	gpipeline_ = graphics_wrapper->CreateGraphicsPipeline(luminanceGPCI);
}

PostProcessBloom::~PostProcessBloom()
{
}

void PostProcessBloom::Process() {
	GRIND_PROFILE_FUNC();
    gpipeline_->Bind();
	engine.getGraphicsWrapper()->BindDefaultFramebuffer(true);
	engine.getGraphicsWrapper()->Clear(Grindstone::GraphicsAPI::ClearMode::Both);
	//target_->framebuffer->BindWrite(true);
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(4);
    engine.getGraphicsWrapper()->DrawImmediateVertices(0, 6);
}

void PostProcessBloom::resizeBuffers(unsigned int w, unsigned h)
{
}

void PostProcessBloom::reloadGraphics(unsigned int w, unsigned h)
{
}

void PostProcessBloom::destroyGraphics()
{
}
