#include "PostProcessSSR.hpp"
#include "GraphicsWrapper.hpp"
#include "../Core/Engine.hpp"
#include <glm/glm.hpp>
#include "Utilities/SettingsFile.hpp"
#include "Core/Utilities.hpp"
#include "PostProcessAutoExposure.hpp"

// , PostProcessAutoExposure *auto_exposure
// , auto_exposure_(auto_exposure)
PostProcessSSR::PostProcessSSR(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target) : BasePostProcess(pipeline), source_(source), target_(target) {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2];

	// SSR Graphics Pipeline
	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/ssr.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/ssr.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/ssr.spv";
	}

	std::vector<char> vfile;
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("SSR Vertex Shader missing.\n");
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("SSR Fragment Shader missing.\n");
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo ssrGPCI;
	ssrGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	ssrGPCI.bindings = &vbd;
	ssrGPCI.bindingsCount = 1;
	ssrGPCI.attributes = &vad;
	ssrGPCI.attributesCount = 1;
	ssrGPCI.width = (float)settings->resolution_x_;
	ssrGPCI.height = (float)settings->resolution_y_;
	ssrGPCI.scissorW = settings->resolution_x_;
	ssrGPCI.scissorH = settings->resolution_y_;
	ssrGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	ssrGPCI.shaderStageCreateInfos = stages;
	ssrGPCI.shaderStageCreateInfoCount = 2;

	Grindstone::GraphicsAPI::TextureSubBinding *ssr_sub_binding_ = new Grindstone::GraphicsAPI::TextureSubBinding("lighting", 4);


	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = ssr_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	Grindstone::GraphicsAPI::TextureBindingLayout *ssr_tbl_ = graphics_wrapper->CreateTextureBindingLayout(tblci);

	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout*> tbls_refl = { engine.gbuffer_tbl_, ssr_tbl_ }; // refl_tbl

	ssrGPCI.textureBindings = tbls_refl.data();
	ssrGPCI.textureBindingCount = (uint32_t)tbls_refl.size();
	ssrGPCI.uniformBufferBindings = nullptr;
	ssrGPCI.uniformBufferBindingCount = 0;
	gpipeline_ = graphics_wrapper->CreateGraphicsPipeline(ssrGPCI);
}

PostProcessSSR::~PostProcessSSR()
{
}

void PostProcessSSR::Process() {
	GRIND_PROFILE_FUNC();
	auto graphics_wrapper = engine.getGraphicsWrapper();
	double dt = engine.getUpdateTimeDelta();

	gpipeline_->Bind();
	if (target_ == nullptr) {
		engine.getGraphicsWrapper()->BindDefaultFramebuffer(true);
		engine.getGraphicsWrapper()->Clear(Grindstone::GraphicsAPI::ClearMode::Both);
	}
	else {
		target_->framebuffer->BindWrite(true);
		//target_->framebuffer->Clear(Grindstone::GraphicsAPI::ClearMode::Both);
	}
	graphics_wrapper->SetImmediateBlending(Grindstone::GraphicsAPI::BlendMode::Additive);

	source_->framebuffer->BindRead();
	source_->framebuffer->BindTextures(4);
	graphics_wrapper->DrawImmediateVertices(0, 6);

	graphics_wrapper->SetImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
}

void PostProcessSSR::resizeBuffers(unsigned int w, unsigned h)
{
}

void PostProcessSSR::reloadGraphics(unsigned int w, unsigned h)
{
}

void PostProcessSSR::destroyGraphics()
{
}
