#include "PostProcessTonemap.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>
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
	GRIND_PROFILE_FUNC();
	float dt = (float)engine.getUpdateTimeDelta();

	if (auto_exposure_) {
		GRIND_PROFILE_SCOPE("Calculating Auto Exposure");
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
	effect_ub_->updateBuffer(&effect_buffer_);

    gpipeline_->Bind();
    effect_ub_->bind();
	if (target_ == nullptr) {
		engine.getGraphicsWrapper()->bindDefaultFramebuffer(true);
		float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		engine.getGraphicsWrapper()->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);
	} 
	else {
		target_->framebuffer->BindWrite(true);
		//target_->framebuffer->Clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
	}
    source_->framebuffer->BindRead();
    source_->framebuffer->BindTextures(4);
    engine.getGraphicsWrapper()->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
}

void PostProcessTonemap::resizeBuffers(unsigned int w, unsigned h)
{
}

void PostProcessTonemap::reloadGraphics(unsigned int w, unsigned h) {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	auto gl = engine.getSettings()->graphics_language_;

	// Exposure Uniform Buffer
	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "EffectUBO";
	ubbci.size = sizeof(EffectUBO);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	ubb_ = graphics_wrapper->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(EffectUBO);
	ubci.binding = ubb_;
	effect_ub_ = graphics_wrapper->createUniformBuffer(ubci);

	effect_ub_->updateBuffer(&effect_buffer_);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2];

	// Tonemap Graphics Pipeline
	if (gl == GraphicsLanguage::OpenGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/tonemap.glsl";
	}
	else if (gl == GraphicsLanguage::DirectX) {
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
	stages[0].type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Tonemapping Fragment Shader missing.\n");
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo tonemapGPCI;
	tonemapGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	tonemapGPCI.vertex_bindings = &vertex_layout;
	tonemapGPCI.vertex_bindings_count = 1;
	tonemapGPCI.width = (float)w;
	tonemapGPCI.height = (float)h;
	tonemapGPCI.scissorW = w;
	tonemapGPCI.scissorH = h;
	tonemapGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	tonemapGPCI.shaderStageCreateInfos = stages;
	tonemapGPCI.shaderStageCreateInfoCount = 2;

	tonemap_sub_binding_ = new Grindstone::GraphicsAPI::TextureSubBinding("lighting", 4);

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = tonemap_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	tonemap_tbl_ = graphics_wrapper->createTextureBindingLayout(tblci);

	tonemapGPCI.textureBindings = &tonemap_tbl_;
	tonemapGPCI.textureBindingCount = 1;
	tonemapGPCI.uniformBufferBindings = &ubb_;
	tonemapGPCI.uniformBufferBindingCount = 1;
	gpipeline_ = graphics_wrapper->createGraphicsPipeline(tonemapGPCI);
}

void PostProcessTonemap::destroyGraphics() {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	if (gpipeline_) {
		graphics_wrapper->deleteGraphicsPipeline(gpipeline_);
		gpipeline_ = nullptr;
	}
	if (effect_ub_) {
		graphics_wrapper->deleteUniformBuffer(effect_ub_);
		effect_ub_ = nullptr;
	}
	if (ubb_) {
		graphics_wrapper->deleteUniformBufferBinding(ubb_);
		ubb_ = nullptr;
	}
	if (tonemap_tbl_) {
		graphics_wrapper->deleteTextureBindingLayout(tonemap_tbl_);
		tonemap_tbl_ = nullptr;
	}

	if (tonemap_sub_binding_) {
		delete tonemap_sub_binding_;
		tonemap_sub_binding_ = nullptr;
	}
}
