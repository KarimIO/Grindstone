#include "RenderPathDeferred.hpp"
#include "Core/Engine.hpp"
#include <stb/stb_image.h>
#include "GraphicsWrapper.hpp"
#include "AssetManagers/GraphicsPipelineManager.hpp"

#include  "../Core/Engine.hpp"
#include "../GraphicsCommon/RenderTarget.hpp"
#include "../GraphicsCommon/DepthTarget.hpp"
#include "../GraphicsCommon/GraphicsWrapper.hpp"

RenderPathDeferred::RenderPathDeferred() {
	createFramebuffer();
	createPointLightShader();
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
	if (engine.settings->graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/ibl.glsl";
	}
	else if (engine.settings->graphicsLanguage == GRAPHICS_DIRECTX) {
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
	iblGPCI.width = (float)engine.settings->resolution_x_;
	iblGPCI.height = (float)engine.settings->resolution_y_;
	iblGPCI.scissorW = engine.settings->resolution_x_;
	iblGPCI.scissorH = engine.settings->resolution_y_;
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

void RenderPathDeferred::createPointLightShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	UniformBufferBindingCreateInfo light_ubbci;
	light_ubbci.binding = 1;
	light_ubbci.shaderLocation = "Light";
	light_ubbci.size = sizeof(LightPointUBO);
	light_ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	point_light_ubb_ = engine.getGraphicsWrapper()->CreateUniformBufferBinding(light_ubbci);

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = point_light_ubb_;
	point_light_ubo_handler_ = graphics_wrapper->CreateUniformBuffer(lightuboci);


	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.spv";
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

	GraphicsPipelineCreateInfo pointGPCI;
	pointGPCI.cullMode = CULL_BACK;
	pointGPCI.bindings = &plane_vbd_;
	pointGPCI.bindingsCount = 1;
	pointGPCI.attributes = &plane_vad_;
	pointGPCI.attributesCount = 1;
	pointGPCI.width = (float)engine.getSettings()->resolution_x_;
	pointGPCI.height = (float)engine.getSettings()->resolution_y_;
	pointGPCI.scissorW = engine.getSettings()->resolution_x_;
	pointGPCI.scissorH = engine.getSettings()->resolution_y_;
	pointGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	pointGPCI.shaderStageCreateInfos = stages.data();
	pointGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	pointGPCI.textureBindings = &gbuffer_tbl_;
	pointGPCI.textureBindingCount = 1;
	std::vector<UniformBufferBinding *> ubbs = { deff_ubb_, point_light_ubb_ };
	pointGPCI.uniformBufferBindings = ubbs.data();
	pointGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	point_light_pipeline_ = engine.getGraphicsWrapper()->CreateGraphicsPipeline(pointGPCI);


	light_point_ubo_.position[0] = 2;
	light_point_ubo_.position[1] = 2;
	light_point_ubo_.position[2] = 0;
	light_point_ubo_.color[0] = 1;
	light_point_ubo_.color[1] = 1;
	light_point_ubo_.color[2] = 1;
	light_point_ubo_.attenuationRadius = 60;
	light_point_ubo_.power = 80;
	light_point_ubo_.shadow = false;
	point_light_ubo_handler_->UpdateUniformBuffer(&light_point_ubo_);
}

void RenderPathDeferred::render(Framebuffer *default, glm::mat4 p, glm::mat4 v, glm::vec3 eye) {
	deferred_ubo_.invProj = glm::inverse(p);
	deferred_ubo_.view = glm::inverse(v);
	deferred_ubo_.eyePos.x = eye.x;
	deferred_ubo_.eyePos.y = eye.y;
	deferred_ubo_.eyePos.z = eye.z;
	deferred_ubo_.resolution.x = engine.getSettings()->resolution_x_;
	deferred_ubo_.resolution.y = engine.getSettings()->resolution_y_;
	deff_ubo_handler_->UpdateUniformBuffer(&deferred_ubo_);

	// Opaque
	gbuffer_->Bind(true);
	gbuffer_->Clear(CLEAR_BOTH);
	engine.getGraphicsWrapper()->SetImmediateBlending(BLEND_NONE);
	engine.getGraphicsPipelineManager()->drawDeferredImmediate();

	deff_ubo_handler_->Bind();

	/*if (engine.debug_wrapper_.GetDebugMode() != 0) {
		engine.debug_wrapper_.Draw();
	}
	else*/
	{
		// Deferred
		renderLights();
		
		
		
		/*gbuffer_->BindRead();
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
		engine.skybox_.Render();*/
		
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

		//CCamera *cam = &engine.cameraSystem.components[0];
		//cam->PostProcessing();
	}
}

void RenderPathDeferred::renderLights() {
	
	engine.getGraphicsWrapper()->SetImmediateBlending(BLEND_ADDITIVE);
	//engine.hdr_framebuffer_->BindWrite(false);
	engine.getGraphicsWrapper()->BindDefaultFramebuffer(true);
	engine.getGraphicsWrapper()->Clear(CLEAR_BOTH);
	gbuffer_->BindRead();
	gbuffer_->BindTextures(0);

	point_light_pipeline_->Bind();
	//for (auto &light : engine.getSystem()) {
		point_light_ubo_handler_->Bind();

		engine.getGraphicsWrapper()->BindVertexArrayObject(plane_vao_);
		engine.getGraphicsWrapper()->DrawImmediateVertices(0, 6);
	//}

	/*engine.lightSystem.m_spotLightPipeline->Bind();
	for (auto &light : engine.lightSystem.spotLights) {
		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}

	engine.lightSystem.m_directionalLightPipeline->Bind();
	for (auto &light : engine.lightSystem.directionalLights) {
		//if (light.cascades_count_ > 1)
		//	engine.lightSystem.m_cascadeLightPipeline->Bind();
		//else

		light.Bind();

		m_graphics_wrapper_->DrawImmediateVertices(0, 6);
	}*/
}

void RenderPathDeferred::createFramebuffer() {
	auto settings = engine.getSettings();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	bindings.reserve(4);
	bindings.emplace_back("gbuffer0", 0); // R G B MatID
	bindings.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	bindings.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	bindings.emplace_back("gbuffer3", 3); // Depth

	UniformBufferBindingCreateInfo deffubbci;
	deffubbci.binding = 0;
	deffubbci.shaderLocation = "UniformBufferObject";
	deffubbci.size = sizeof(DefferedUBO);
	deffubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	deff_ubb_ = graphics_wrapper->CreateUniformBufferBinding(deffubbci);

	UniformBufferCreateInfo deffubci;
	deffubci.isDynamic = false;
	deffubci.size = sizeof(DefferedUBO);
	deffubci.binding = deff_ubb_;
	deff_ubo_handler_ = graphics_wrapper->CreateUniformBuffer(deffubci);

	float plane_verts[4 * 6] = {
		-1.0, -1.0,
		1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		-1.0,  1.0,
		1.0, -1.0,
	};

	plane_vbd_.binding = 0;
	plane_vbd_.elementRate = false;
	plane_vbd_.stride = sizeof(float) * 2;

	plane_vad_.binding = 0;
	plane_vad_.location = 0;
	plane_vad_.format = VERTEX_R32_G32;
	plane_vad_.size = 2;
	plane_vad_.name = "vertexPosition";
	plane_vad_.offset = 0;
	plane_vad_.usage = ATTRIB_POSITION;


	VertexBufferCreateInfo planeVboCI;
	planeVboCI.binding = &plane_vbd_;
	planeVboCI.bindingCount = 1;
	planeVboCI.attribute = &plane_vad_;
	planeVboCI.attributeCount = 1;
	planeVboCI.content = plane_verts;
	planeVboCI.count = 6;
	planeVboCI.size = sizeof(float) * 6 * 2;

	VertexArrayObjectCreateInfo plane_vao_ci;
	plane_vao_ci.vertexBuffer = plane_vbo_;
	plane_vao_ci.indexBuffer = nullptr;
	plane_vao_ = graphics_wrapper->CreateVertexArrayObject(plane_vao_ci);
	plane_vbo_ = graphics_wrapper->CreateVertexBuffer(planeVboCI);
	plane_vao_ci.vertexBuffer = plane_vbo_;
	plane_vao_ci.indexBuffer = nullptr;
	plane_vao_->BindResources(plane_vao_ci);
	plane_vao_->Unbind();

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = bindings.data();
	tblci.bindingCount = (uint32_t)bindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	gbuffer_tbl_ = graphics_wrapper->CreateTextureBindingLayout(tblci);

	std::vector<RenderTargetCreateInfo> gbuffer_images_ci;
	gbuffer_images_ci.reserve(3);
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings->resolution_x_, settings->resolution_y_); // R  G  B matID
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R16G16B16A16, settings->resolution_x_, settings->resolution_y_); // nX nY nZ
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings->resolution_x_, settings->resolution_y_); // sR sG sB Roughness
	render_targets_ = graphics_wrapper->CreateRenderTarget(gbuffer_images_ci.data(), gbuffer_images_ci.size());

	DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24_STENCIL_8, settings->resolution_x_, settings->resolution_y_, false, false);
	depth_target_ = graphics_wrapper->CreateDepthTarget(depth_image_ci);

	FramebufferCreateInfo gbuffer_ci;
	gbuffer_ci.render_target_lists = &render_targets_;
	gbuffer_ci.num_render_target_lists = 1;
	gbuffer_ci.depth_target = depth_target_;
	gbuffer_ci.render_pass = nullptr;
	gbuffer_ = graphics_wrapper->CreateFramebuffer(gbuffer_ci);

}
