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

void RenderPathDeferred::Render(Framebuffer *gbuffer_) {
	// Opaque
	gbuffer_->Bind(true);
	gbuffer_->Clear(CLEAR_BOTH);
	m_graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
	engine.materialManager.DrawDeferredImmediate();

	if (engine.debug_wrapper_.GetDebugMode() == 0) {
		// Deferred
		RenderLights(gbuffer_);
		gbuffer_->BindRead();
		engine.hdr_framebuffer_->BindWrite(true);
		m_graphics_wrapper_->CopyToDepthBuffer(engine.depth_image_);
		engine.hdr_framebuffer_->BindRead();

		m_graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
		// Unlit
		engine.materialManager.DrawUnlitImmediate();	
		// Forward
		m_graphics_wrapper_->SetImmediateBlending(BLEND_ADD_ALPHA);
		engine.ubo->Bind();
		engine.ubo2->Bind();
		engine.materialManager.DrawForwardImmediate();
		m_graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
		engine.deffUBO->Bind();
		engine.skybox_.Render();
		
		/*graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
		pipeline_ssr_->Bind();
		hdr_framebuffer_->Bind(false);
		hdr_framebuffer_->BindTextures(4);
		deffUBO->Bind();
		gbuffer_->BindRead();
		gbuffer_->BindTextures(0);
		graphics_wrapper_->DrawImmediateVertices(0, 6);

		//exposure_buffer_.exposure = hdr_framebuffer_->getExposure(0);
		//exposure_buffer_.exposure = exposure_buffer_.exposure*100.0f/12.5f;
		//std::cout << exposure_buffer_.exposure << "\n";
		//exposure_ub_->UpdateUniformBuffer(&exposure_buffer_);
		
		/*graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
		pipeline_bloom_->Bind();
		hdr_framebuffer_->Bind(false);
		hdr_framebuffer_->BindTextures(0);
		graphics_wrapper_->DrawImmediateVertices(0, 6);*/

		/*pipeline_tonemap_->Bind();
		exposure_ub_->Bind();
		graphics_wrapper_->BindDefaultFramebuffer(false);
		graphics_wrapper_->Clear(CLEAR_BOTH);
		hdr_framebuffer_->BindRead();
		hdr_framebuffer_->BindTextures(4);
		graphics_wrapper_->DrawImmediateVertices(0, 6);*/

		CCamera *cam = &engine.cameraSystem.components[0];
		cam->PostProcessing();
	}
	else {
		engine.debug_wrapper_.Draw();
	}
}

void RenderPathDeferred::RenderLights(Framebuffer *gbuffer) {
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
