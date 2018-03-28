#include "RenderPathDeferred.hpp"
#include "Core/Engine.hpp"
#include <stb/stb_image.h>

RenderPathDeferred::RenderPathDeferred(GraphicsWrapper * graphics_wrapper_, VertexArrayObject *plane_vao) {
	m_graphics_wrapper_ = graphics_wrapper_;

	plane_vao_ = plane_vao;
	
	/*
	//=====================
	// SSAO Blur
	//=====================
	std::vector<TextureSubBinding> cubemapBindings;
	cubemapBindings.reserve(1);
	cubemapBindings.emplace_back("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 1;
	tblci.bindings = cubemapBindings.data();
	tblci.bindingCount = (uint32_t)cubemapBindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *cubemapLayout = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = m_cubemap;
	stb.address = 4;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.layout = cubemapLayout;
	ci.textureCount = 1;
	m_cubemapBinding = m_graphics_wrapper_->CreateTextureBinding(ci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.spv";
	}
	std::vector<char> vfile;
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<ShaderStageCreateInfo> stages = { vi, fi };

	GraphicsPipelineCreateInfo iblGPCI;
	iblGPCI.cullMode = CULL_BACK;
	iblGPCI.bindings = &engine.planeVBD;
	iblGPCI.bindingsCount = 1;
	iblGPCI.attributes = &engine.planeVAD;
	iblGPCI.attributesCount = 1;
	iblGPCI.width = (float)engine.settings.resolutionX;
	iblGPCI.height = (float)engine.settings.resolutionY;
	iblGPCI.scissorW = engine.settings.resolutionX;
	iblGPCI.scissorH = engine.settings.resolutionY;
	iblGPCI.primitiveType = PRIM_TRIANGLES;
	iblGPCI.shaderStageCreateInfos = stages.data();
	iblGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout *> tbls = { engine.tbl, cubemapLayout };

	iblGPCI.textureBindings = tbls.data();
	iblGPCI.textureBindingCount = (uint32_t)tbls.size();
	iblGPCI.uniformBufferBindings = &engine.deffubb;
	iblGPCI.uniformBufferBindingCount = 1;
	m_iblPipeline = graphics_wrapper_->CreateGraphicsPipeline(iblGPCI);*/
}

void RenderPathDeferred::Draw(Framebuffer *gbuffer) {
	engine.deffUBO->Bind();
	engine.graphics_wrapper_->BindVertexArrayObject(plane_vao_);
	
	m_graphics_wrapper_->SetImmediateBlending(BLEND_ADDITIVE);
	engine.hdr_framebuffer_->BindWrite(false);
	m_graphics_wrapper_->Clear(CLEAR_BOTH);
	//engine.graphics_wrapper_->BindDefaultFramebuffer(false);
	gbuffer->BindRead();
	gbuffer->BindTextures(0);

	engine.lightSystem.m_pointLightPipeline->Bind();
	for (auto &light : engine.lightSystem.pointLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	engine.lightSystem.m_spotLightPipeline->Bind();
	for (auto &light : engine.lightSystem.spotLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	engine.lightSystem.m_directionalLightPipeline->Bind();
	for (auto &light : engine.lightSystem.directionalLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}
}
