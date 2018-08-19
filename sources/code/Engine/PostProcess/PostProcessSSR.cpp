#include "PostProcessSSR.hpp"
#include "../Core/Engine.hpp"
#include "../../GraphicsCommon/UniformBuffer.hpp"

PostProcessSSR::PostProcessSSR(RenderTargetContainer *source, RenderTargetContainer *target) : source_(source), target_(target) {
    GraphicsWrapper *graphics_wrapper_ = engine.graphics_wrapper_;

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/post_processing/ssr.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/post_processing/ssr.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/post_processing/ssr.spv";
	}
	
	vfile.clear();
	if (!readFile(vi.fileName, vfile)) {
		throw std::runtime_error("SSR Vertex Shader missing.\n");
		return;
	}
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("SSR Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	ShaderStageCreateInfo stages[2] = { vi, fi };

	GraphicsPipelineCreateInfo SSRGPCI;
	SSRGPCI.cullMode = CULL_BACK;
	SSRGPCI.bindings = &engine.planeVBD;
	SSRGPCI.bindingsCount = 1;
	SSRGPCI.attributes = &engine.planeVAD;
	SSRGPCI.attributesCount = 1;
	SSRGPCI.width = (float)engine.settings.resolutionX; // DIVIDE BY TWO
	SSRGPCI.height = (float)engine.settings.resolutionY;
	SSRGPCI.scissorW = engine.settings.resolutionX;
	SSRGPCI.scissorH = engine.settings.resolutionY;
	SSRGPCI.primitiveType = PRIM_TRIANGLES;
	SSRGPCI.shaderStageCreateInfos = stages;
	SSRGPCI.shaderStageCreateInfoCount = 2;
	std::vector<TextureBindingLayout *>tbls = { engine.tbl, engine.tonemap_tbl_	 };

	std::vector<UniformBufferBinding*> ubbs = { engine.deffubb };
	SSRGPCI.textureBindings = tbls.data();
	SSRGPCI.textureBindingCount = (uint32_t)tbls.size();
	SSRGPCI.uniformBufferBindings = ubbs.data();
	SSRGPCI.uniformBufferBindingCount = ubbs.size();
	pipeline_ = graphics_wrapper_->CreateGraphicsPipeline(SSRGPCI);
}

void PostProcessSSR::Process() {// BLEND_ADDITIVE
	engine.graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
	target_->framebuffer->BindWrite(true);
	source_->framebuffer->BindRead();
	source_->framebuffer->BindTextures(0);

	//target_->framebuffer->BindRead();
	//target_->framebuffer->BindTextures(4);
	
	//if (target_ == nullptr) {
		engine.graphics_wrapper_->BindDefaultFramebuffer(false);
		engine.graphics_wrapper_->Clear(CLEAR_BOTH);
	//}

	pipeline_->Bind();
	engine.deffUBO->Bind();
	engine.graphics_wrapper_->DrawImmediateVertices(0, 6);
	engine.graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
}