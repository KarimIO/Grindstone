#include "PostProcessTonemap.hpp"
#include "../Core/Engine.hpp"

PostProcessTonemap::PostProcessTonemap(RenderTargetContainer *source, RenderTargetContainer *target) : source_(source), target_(target) {
    GraphicsWrapper *graphics_wrapper_ = engine.graphics_wrapper_;
    
    // Exposure Uniform Buffer
    UniformBufferBindingCreateInfo ubbci;
    ubbci.binding = 0;
	ubbci.shaderLocation = "ExposureUBO";
	ubbci.size = sizeof(ExposureUBO);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	UniformBufferBinding *ubb = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

    UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(ExposureUBO);
	ubci.binding = ubb;
	exposure_ub_ = graphics_wrapper_->CreateUniformBuffer(ubci);

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
	pipeline_ = graphics_wrapper_->CreateGraphicsPipeline(tonemapGPCI);
}

void PostProcessTonemap::Process() {
    pipeline_->Bind();
    exposure_ub_->Bind();
	if (target_ == nullptr) {
		engine.graphics_wrapper_->BindDefaultFramebuffer(false);
		engine.graphics_wrapper_->Clear(CLEAR_BOTH);
	}
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(0);
    engine.graphics_wrapper_->DrawImmediateVertices(0, 6);
}