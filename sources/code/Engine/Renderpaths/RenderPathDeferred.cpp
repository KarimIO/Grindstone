#include "RenderPathDeferred.hpp"
#include "Core/Engine.hpp"
#include <stb/stb_image.h>
#include "GraphicsWrapper.hpp"
#include "AssetManagers/GraphicsPipelineManager.hpp"

#include  "../Core/Engine.hpp"
#include "../GraphicsCommon/RenderTarget.hpp"
#include "../GraphicsCommon/DepthTarget.hpp"
#include "../GraphicsCommon/GraphicsWrapper.hpp"

#include "../Systems/TransformSystem.hpp"
#include "../Systems/LightPointSystem.hpp"
#include "../Systems/LightSpotSystem.hpp"
#include "../Systems/LightDirectionalSystem.hpp"

#include "Core/Space.hpp"

glm::mat4 bias_matrix(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

RenderPathDeferred::RenderPathDeferred(unsigned int w = engine.getSettings()->resolution_x_, unsigned int h = engine.getSettings()->resolution_y_) {
	auto settings = engine.getSettings();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	shadow_binding_ = TextureSubBinding("shadow_map", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &shadow_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	shadow_tbl_ = graphics_wrapper->CreateTextureBindingLayout(tblci);

	createFramebuffer(w, h);
	createPointLightShader();
	createSpotLightShader();
	createDirectionalLightShader();

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

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	GraphicsPipelineCreateInfo pointGPCI;
	pointGPCI.cullMode = CULL_BACK;
	pointGPCI.bindings = &vbd;
	pointGPCI.bindingsCount = 1;
	pointGPCI.attributes = &vad;
	pointGPCI.attributesCount = 1;
	pointGPCI.width = (float)engine.getSettings()->resolution_x_;
	pointGPCI.height = (float)engine.getSettings()->resolution_y_;
	pointGPCI.scissorW = engine.getSettings()->resolution_x_;
	pointGPCI.scissorH = engine.getSettings()->resolution_y_;
	pointGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	pointGPCI.shaderStageCreateInfos = stages.data();
	pointGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout *> tbls_ = { engine.gbuffer_tbl_, shadow_tbl_ };
	pointGPCI.textureBindings = tbls_.data();
	pointGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<UniformBufferBinding *> ubbs = { engine.deff_ubb_, point_light_ubb_ };
	pointGPCI.uniformBufferBindings = ubbs.data();
	pointGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	point_light_pipeline_ = engine.getGraphicsWrapper()->CreateGraphicsPipeline(pointGPCI);
}

void RenderPathDeferred::createSpotLightShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	UniformBufferBindingCreateInfo light_ubbci;
	light_ubbci.binding = 1;
	light_ubbci.shaderLocation = "Light";
	light_ubbci.size = sizeof(LightSpotUBO);
	light_ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	spot_light_ubb_ = engine.getGraphicsWrapper()->CreateUniformBufferBinding(light_ubbci);

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = spot_light_ubb_;
	spot_light_ubo_handler_ = graphics_wrapper->CreateUniformBuffer(lightuboci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.spv";
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

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	GraphicsPipelineCreateInfo spotGPCI;
	spotGPCI.cullMode = CULL_BACK;
	spotGPCI.bindings = &vbd;
	spotGPCI.bindingsCount = 1;
	spotGPCI.attributes = &vad;
	spotGPCI.attributesCount = 1;
	spotGPCI.width = (float)engine.getSettings()->resolution_x_;
	spotGPCI.height = (float)engine.getSettings()->resolution_y_;
	spotGPCI.scissorW = engine.getSettings()->resolution_x_;
	spotGPCI.scissorH = engine.getSettings()->resolution_y_;
	spotGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	spotGPCI.shaderStageCreateInfos = stages.data();
	spotGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout *> tbls_ = { engine.gbuffer_tbl_, shadow_tbl_ };
	spotGPCI.textureBindings = tbls_.data();
	spotGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<UniformBufferBinding *> ubbs = { engine.deff_ubb_, spot_light_ubb_ };
	spotGPCI.uniformBufferBindings = ubbs.data();
	spotGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	spot_light_pipeline_ = engine.getGraphicsWrapper()->CreateGraphicsPipeline(spotGPCI);
}

void RenderPathDeferred::createDirectionalLightShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	UniformBufferBindingCreateInfo light_ubbci;
	light_ubbci.binding = 1;
	light_ubbci.shaderLocation = "Light";
	light_ubbci.size = sizeof(LightDirectionalUBO);
	light_ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	directional_light_ubb_ = engine.getGraphicsWrapper()->CreateUniformBufferBinding(light_ubbci);

	UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = false;
	lightuboci.size = sizeof(LightDirectionalUBO);
	lightuboci.binding = directional_light_ubb_;
	directional_light_ubo_handler_ = graphics_wrapper->CreateUniformBuffer(lightuboci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.fxc";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.spv";
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

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	GraphicsPipelineCreateInfo directionalGPCI;
	directionalGPCI.cullMode = CULL_BACK;
	directionalGPCI.bindings = &vbd;
	directionalGPCI.bindingsCount = 1;
	directionalGPCI.attributes = &vad;
	directionalGPCI.attributesCount = 1;
	directionalGPCI.width = (float)engine.getSettings()->resolution_x_;
	directionalGPCI.height = (float)engine.getSettings()->resolution_y_;
	directionalGPCI.scissorW = engine.getSettings()->resolution_x_;
	directionalGPCI.scissorH = engine.getSettings()->resolution_y_;
	directionalGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	directionalGPCI.shaderStageCreateInfos = stages.data();
	directionalGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout *> tbls_ = { engine.gbuffer_tbl_, shadow_tbl_ };
	directionalGPCI.textureBindings = tbls_.data();
	directionalGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<UniformBufferBinding *> ubbs = { engine.deff_ubb_, directional_light_ubb_ };
	directionalGPCI.uniformBufferBindings = ubbs.data();
	directionalGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	directional_light_pipeline_ = engine.getGraphicsWrapper()->CreateGraphicsPipeline(directionalGPCI);
}

void RenderPathDeferred::render(Framebuffer *fbo, Space *space) {
	bool debug = false;
	
	// Opaque
	if (debug) {
		engine.getGraphicsWrapper()->BindDefaultFramebuffer(true);
		engine.getGraphicsWrapper()->Clear(CLEAR_BOTH);
	}
	else {
		gbuffer_->Bind(true);
		gbuffer_->Clear(CLEAR_BOTH);
	}

	engine.getGraphicsWrapper()->SetImmediateBlending(BLEND_NONE);
	engine.getGraphicsPipelineManager()->drawDeferredImmediate();


	if (!debug)
		engine.deff_ubo_handler_->Bind();

	/*if (engine.debug_wrapper_.GetDebugMode() != 0) {
		engine.debug_wrapper_.Draw();
	}
	else*/
	{
		// Deferred
		if (!debug)
			renderLights(fbo, space);
		
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

void RenderPathDeferred::renderLights(Framebuffer *fbo, Space *space) {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	TransformSubSystem *transform_system = (TransformSubSystem *)space->getSubsystem(COMPONENT_TRANSFORM);

	if (fbo) {
		fbo->BindWrite(true);
		fbo->Clear(CLEAR_BOTH);
	}
	else {
		graphics_wrapper->BindDefaultFramebuffer(true);
		graphics_wrapper->Clear(CLEAR_BOTH);
	}
	
	graphics_wrapper->SetImmediateBlending(BLEND_ADDITIVE);
	gbuffer_->BindRead();
	gbuffer_->BindTextures(0);

	LightPointSubSystem *point_light_system = (LightPointSubSystem *)space->getSubsystem(COMPONENT_LIGHT_POINT);
	if (point_light_system->getNumComponents()) {
		point_light_pipeline_->Bind();
		for (size_t i = 0; i < point_light_system->getNumComponents(); ++i) {
			auto &light = point_light_system->getComponent(i);
			GameObjectHandle game_object_handle = light.game_object_handle_;
			ComponentHandle component_transform_handle = space->getObject(game_object_handle).getComponentHandle(COMPONENT_TRANSFORM);

			light_point_ubo_.position = transform_system->getPosition(component_transform_handle);
			light_point_ubo_.color = light.properties_.color;
			light_point_ubo_.attenuationRadius = light.properties_.attenuationRadius;
			light_point_ubo_.power = light.properties_.power;
			light_point_ubo_.shadow = light.properties_.shadow;
			point_light_ubo_handler_->UpdateUniformBuffer(&light_point_ubo_);

			point_light_ubo_handler_->Bind();

			graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());
			graphics_wrapper->DrawImmediateVertices(0, 6);
		}
	}

	LightSpotSubSystem *spot_light_system = (LightSpotSubSystem *)space->getSubsystem(COMPONENT_LIGHT_SPOT);
	if (spot_light_system->getNumComponents() > 0) {
		spot_light_pipeline_->Bind();
		for (size_t i = 0; i < spot_light_system->getNumComponents(); ++i) {
			auto &light = spot_light_system->getComponent(i);
			GameObjectHandle game_object_handle = light.game_object_handle_;
			ComponentHandle component_transform_handle = space->getObject(game_object_handle).getComponentHandle(COMPONENT_TRANSFORM);

			light_spot_ubo_.direction = transform_system->getForward(component_transform_handle);
			light_spot_ubo_.position = transform_system->getPosition(component_transform_handle);
			light_spot_ubo_.color = light.properties_.color;
			light_spot_ubo_.attenuationRadius = light.properties_.attenuationRadius;
			light_spot_ubo_.power = light.properties_.power;
			light_spot_ubo_.innerAngle = light.properties_.innerAngle;
			light_spot_ubo_.outerAngle = light.properties_.outerAngle;
			light_spot_ubo_.shadow = light.properties_.shadow;
			light_spot_ubo_.shadow_mat = bias_matrix * light.shadow_mat_;
			light_spot_ubo_.shadow_resolution = light.properties_.resolution;
			spot_light_ubo_handler_->UpdateUniformBuffer(&light_spot_ubo_);

			if (light.properties_.shadow) {
				light.shadow_fbo_->BindRead();
				light.shadow_fbo_->BindTextures(4);
			}

			spot_light_ubo_handler_->Bind();

			graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());
			graphics_wrapper->DrawImmediateVertices(0, 6);
		}
	}

	LightDirectionalSubSystem *directional_light_system = (LightDirectionalSubSystem *)space->getSubsystem(COMPONENT_LIGHT_DIRECTIONAL);
	if (directional_light_system->getNumComponents() > 0) {
		directional_light_pipeline_->Bind();
		for (size_t i = 0; i < directional_light_system->getNumComponents(); ++i) {
			auto &light = directional_light_system->getComponent(i);
			GameObjectHandle game_object_handle = light.game_object_handle_;
			ComponentHandle component_transform_handle = space->getObject(game_object_handle).getComponentHandle(COMPONENT_TRANSFORM);

			light_directional_ubo_.direction = transform_system->getForward(component_transform_handle);
			light_directional_ubo_.color = light.properties_.color;
			light_directional_ubo_.source_radius = light.properties_.sourceRadius;
			light_directional_ubo_.power = light.properties_.power;
			light_directional_ubo_.shadow = light.properties_.shadow;
			light_directional_ubo_.shadow_mat = bias_matrix * light.shadow_mat_;
			light_directional_ubo_.shadow_resolution = light.properties_.resolution;
			directional_light_ubo_handler_->UpdateUniformBuffer(&light_directional_ubo_);

			if (light.properties_.shadow) {
				light.shadow_fbo_->BindRead();
				light.shadow_fbo_->BindTextures(4);
			}

			directional_light_ubo_handler_->Bind();

			graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());
			graphics_wrapper->DrawImmediateVertices(0, 6);
		}
	}
}

void RenderPathDeferred::createFramebuffer(unsigned int width, unsigned int height) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	std::vector<RenderTargetCreateInfo> gbuffer_images_ci;
	gbuffer_images_ci.reserve(3);
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, width, height); // R  G  B matID
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R16G16B16A16, width, height); // nX nY nZ
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, width, height); // sR sG sB Roughness
	render_targets_ = graphics_wrapper->CreateRenderTarget(gbuffer_images_ci.data(), gbuffer_images_ci.size());

	DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24_STENCIL_8, width, height, false, false);
	depth_target_ = graphics_wrapper->CreateDepthTarget(depth_image_ci);

	FramebufferCreateInfo gbuffer_ci;
	gbuffer_ci.render_target_lists = &render_targets_;
	gbuffer_ci.num_render_target_lists = 1;
	gbuffer_ci.depth_target = depth_target_;
	gbuffer_ci.render_pass = nullptr;
	gbuffer_ = graphics_wrapper->CreateFramebuffer(gbuffer_ci);

}
