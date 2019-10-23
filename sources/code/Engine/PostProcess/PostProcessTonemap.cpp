#include "PostProcessTonemap.hpp"
#include "GraphicsWrapper.hpp"
#include "../Core/Engine.hpp"
#include <glm/glm.hpp>
#include "Utilities/SettingsFile.hpp"
#include "Core/Utilities.hpp"
#include "PostProcessAutoExposure.hpp"

// , PostProcessAutoExposure *auto_exposure
// , auto_exposure_(auto_exposure)
PostProcessTonemap::PostProcessTonemap(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure) : BasePostProcess(pipeline), source_(source), target_(target), auto_exposure_(auto_exposure) {
	effect_buffer_.vignetteRadius = 0.75f;
	effect_buffer_.vignetteSoftness = 0.45f;
	effect_buffer_.vignetteStrength = 0.5f;
	effect_buffer_.noiseStrength = 2.0f;
	effect_buffer_.exposure = exp(0.0f);
	first_render_ = true;

	reloadGraphics(w, h);
}

void PostProcessTonemap::Process() {
	float dt = (float)engine.getUpdateTimeDelta();

	if (auto_exposure_) {
		if (first_render_) {
			effect_buffer_.exposure = 1.0f / glm::clamp((float)exp(auto_exposure_->GetExposure()), 0.1f, 16.0f);
			
			first_render_ = false;
		}
		else {
			float new_exp = 1.0f / glm::clamp((float)exp(auto_exposure_->GetExposure()), 0.1f, 16.0f);
			effect_buffer_.exposure = effect_buffer_.exposure * (1.0f - dt) + new_exp * dt;
		}
	}
	else {
		effect_buffer_.exposure = 1.0f;
	}

	effect_buffer_.time = (float)engine.getTimeCurrent();
	effect_ub_->UpdateUniformBuffer(&effect_buffer_);

    gpipeline_->Bind();
    effect_ub_->Bind();
	if (target_ == nullptr) {
		engine.getGraphicsWrapper()->BindDefaultFramebuffer(true);
		engine.getGraphicsWrapper()->Clear(CLEAR_BOTH);
	} 
	else {
		target_->framebuffer->BindWrite(true);
		//target_->framebuffer->Clear(CLEAR_BOTH);
	}
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(4);
    engine.getGraphicsWrapper()->DrawImmediateVertices(0, 6);
}

void PostProcessTonemap::resizeBuffers(unsigned int w, unsigned h)
{
}

void PostProcessTonemap::reloadGraphics(unsigned int w, unsigned h) {
	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto gl = engine.getSettings()->graphics_language_;

	// Exposure Uniform Buffer
	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "EffectUBO";
	ubbci.size = sizeof(EffectUBO);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	ubb_ = graphics_wrapper->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(EffectUBO);
	ubci.binding = ubb_;
	effect_ub_ = graphics_wrapper->CreateUniformBuffer(ubci);

	effect_ub_->UpdateUniformBuffer(&effect_buffer_);

	ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (gl == GRAPHICS_OPENGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/tonemap.glsl";
	}
	else if (gl == GRAPHICS_DIRECTX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/tonemap.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/tonemap.spv";
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

	GraphicsPipelineCreateInfo tonemapGPCI;
	tonemapGPCI.cullMode = CULL_BACK;
	tonemapGPCI.bindings = &vbd;
	tonemapGPCI.bindingsCount = 1;
	tonemapGPCI.attributes = &vad;
	tonemapGPCI.attributesCount = 1;
	tonemapGPCI.width = (float)w;
	tonemapGPCI.height = (float)h;
	tonemapGPCI.scissorW = w;
	tonemapGPCI.scissorH = h;
	tonemapGPCI.primitiveType = PRIM_TRIANGLES;
	tonemapGPCI.shaderStageCreateInfos = stages;
	tonemapGPCI.shaderStageCreateInfoCount = 2;

	tonemap_sub_binding_ = new TextureSubBinding("lighting", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = tonemap_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	tonemap_tbl_ = graphics_wrapper->CreateTextureBindingLayout(tblci);

	tonemapGPCI.textureBindings = &tonemap_tbl_;
	tonemapGPCI.textureBindingCount = 1;
	tonemapGPCI.uniformBufferBindings = &ubb_;
	tonemapGPCI.uniformBufferBindingCount = 1;
	gpipeline_ = graphics_wrapper->CreateGraphicsPipeline(tonemapGPCI);
}

void PostProcessTonemap::destroyGraphics() {
	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	if (gpipeline_) {
		graphics_wrapper->DeleteGraphicsPipeline(gpipeline_);
		gpipeline_ = nullptr;
	}
	if (effect_ub_) {
		graphics_wrapper->DeleteUniformBuffer(effect_ub_);
		effect_ub_ = nullptr;
	}
	if (ubb_) {
		graphics_wrapper->DeleteUniformBufferBinding(ubb_);
		ubb_ = nullptr;
	}
	if (tonemap_tbl_) {
		graphics_wrapper->DeleteTextureBindingLayout(tonemap_tbl_);
		tonemap_tbl_ = nullptr;
	}

	if (tonemap_sub_binding_) {
		delete tonemap_sub_binding_;
		tonemap_sub_binding_ = nullptr;
	}
}
