/*#include "PostProcessTonemap.hpp"
#include "GraphicsWrapper.hpp"
#include "../Core/Engine.hpp"

PostProcessTonemap::PostProcessTonemap(RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure) : source_(source), target_(target), auto_exposure_(auto_exposure) {
    GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
    
    // Exposure Uniform Buffer
    UniformBufferBindingCreateInfo ubbci;
    ubbci.binding = 0;
	ubbci.shaderLocation = "ExposureUBO";
	ubbci.size = sizeof(ExposureUBO);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	UniformBufferBinding *ubb = graphics_wrapper->CreateUniformBufferBinding(ubbci);

    UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(ExposureUBO);
	ubci.binding = ubb;
	exposure_ub_ = graphics_wrapper->CreateUniformBuffer(ubci);

    exposure_buffer_.exposure = exp(0.0f);
	exposure_ub_->UpdateUniformBuffer(&exposure_buffer_);

	ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/tonemap.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
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

	GraphicsPipelineCreateInfo tonemapGPCI;
	tonemapGPCI.cullMode = CULL_BACK;
	tonemapGPCI.bindings = &engine.planeVBD;
	tonemapGPCI.bindingsCount = 1;
	tonemapGPCI.attributes = &engine.planeVAD;
	tonemapGPCI.attributesCount = 1;
	tonemapGPCI.width = (float)engine.settings.resolutionX;
	tonemapGPCI.height = (float)engine.settings.resolutionY;
	tonemapGPCI.scissorW = engine.settings.resolutionX;
	tonemapGPCI.scissorH = engine.settings.resolutionY;
	tonemapGPCI.primitiveType = PRIM_TRIANGLES;
	tonemapGPCI.shaderStageCreateInfos = stages;
	tonemapGPCI.shaderStageCreateInfoCount = 2;

	tonemapGPCI.textureBindings = &engine.tonemap_tbl_;
	tonemapGPCI.textureBindingCount = 1;
	tonemapGPCI.uniformBufferBindings = &ubb;
	tonemapGPCI.uniformBufferBindingCount = 1;
	pipeline_ = graphics_wrapper->CreateGraphicsPipeline(tonemapGPCI);
}

void PostProcessTonemap::Process() {
	double dt = engine.GetRenderTimeDelta();

	float new_exp = 1.0f / glm::clamp(exp(auto_exposure_->GetExposure()), 0.1f, 16.0f);
	exposure_buffer_.exposure = exposure_buffer_.exposure * (1.0f - dt) + new_exp * dt;
	exposure_ub_->UpdateUniformBuffer(&exposure_buffer_);

    pipeline_->Bind();
    exposure_ub_->Bind();
	if (target_ == nullptr) {
		engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);
		engine.getGraphicsWrapper()->Clear(CLEAR_BOTH);
	}
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(4);
    engine.getGraphicsWrapper()->DrawImmediateVertices(0, 6);
}*/