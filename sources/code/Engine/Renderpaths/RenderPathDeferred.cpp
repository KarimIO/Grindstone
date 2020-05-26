#include "RenderPathDeferred.hpp"
#include "Core/Engine.hpp"
#include <stb/stb_image.h>
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "AssetManagers/GraphicsPipelineManager.hpp"

#include  "../Core/Engine.hpp"
#include "../GraphicsCommon/RenderTarget.hpp"
#include "../GraphicsCommon/DepthTarget.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>

#include "../Systems/TransformSystem.hpp"
#include "../Systems/LightPointSystem.hpp"
#include "../Systems/LightSpotSystem.hpp"
#include "../Systems/LightDirectionalSystem.hpp"

#include "Core/Space.hpp"

#include <GL/gl3w.h>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 bias_matrix(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

std::array<glm::vec3, 8> cube_vertices = {
	glm::vec3(-1.0, -1.0,  1.0), glm::vec3( 1.0, -1.0,  1.0),
	glm::vec3( 1.0,  1.0,  1.0), glm::vec3(-1.0,  1.0,  1.0),
	glm::vec3(-1.0, -1.0, -1.0), glm::vec3( 1.0, -1.0, -1.0),
	glm::vec3( 1.0,  1.0, -1.0), glm::vec3(-1.0,  1.0, -1.0)
};

std::array<uint32_t, 36> cube_indices = {
	// front
	2, 1, 0,
	0, 3, 2,
	// right
	6, 5, 1,
	1, 2, 6,
	// back
	5, 6, 7,
	7, 4, 5,
	// left
	3, 0, 4,
	4, 7, 3,
	// bottom
	1, 5, 4,
	4, 0, 1,
	// top
	6, 2, 3,
	3, 7, 6
};

struct SHBuffer {
	glm::mat4 pvm;
	glm::vec3 sh0[9];
	glm::vec3 sh1[9];
} sh_buffer;

Grindstone::GraphicsAPI::VertexArrayObject* cube_vao_;
Grindstone::GraphicsAPI::VertexBuffer* cube_vbo_;
Grindstone::GraphicsAPI::IndexBuffer* cube_ibo_;
Grindstone::GraphicsAPI::VertexBufferLayout cube_vertex_layout_;
Grindstone::GraphicsAPI::GraphicsPipeline *sh_pipeline;
Grindstone::GraphicsAPI::UniformBufferBinding* sh_ubb;
Grindstone::GraphicsAPI::UniformBuffer* sh_ubo;

void RenderPathDeferred::createShadowTextureBindingLayout() {
	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &shadow_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	shadow_tbl_ = engine.getGraphicsWrapper()->createTextureBindingLayout(tblci);
}

RenderPathDeferred::RenderPathDeferred(unsigned int w = engine.getSettings()->resolution_x_, unsigned int h = engine.getSettings()->resolution_y_) {
	auto settings = engine.getSettings();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	shadow_binding_ = Grindstone::GraphicsAPI::TextureSubBinding("shadow_map", 4);

	CreateFramebuffer(w, h);
	createShadowTextureBindingLayout();
	createPointLightShader();
	createSpotLightShader();
	createDirectionalLightShader();
	createDebugShader();
	wireframe_ = false;

	debug_mode_ = 0;

	cube_vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
		});

	Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void*>(cube_indices.data());
	ibci.count = static_cast<uint32_t>(cube_indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * cube_indices.size());
	cube_ibo_ = graphics_wrapper->createIndexBuffer(ibci);

	Grindstone::GraphicsAPI::VertexBufferCreateInfo cube_vbo_ci;
	cube_vbo_ci.layout = &cube_vertex_layout_;
	cube_vbo_ci.content = cube_vertices.data();
	cube_vbo_ci.count = (uint32_t)cube_vertices.size();
	cube_vbo_ci.size = (uint32_t)(sizeof(glm::vec3) * cube_vertices.size());
	cube_vbo_ = graphics_wrapper->createVertexBuffer(cube_vbo_ci);

	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo cube_vao_ci;
	cube_vao_ci.vertex_buffers = &cube_vbo_;
	cube_vao_ci.vertex_buffer_count = 1;
	cube_vao_ci.index_buffer = cube_ibo_;
	cube_vao_ = graphics_wrapper->createVertexArrayObject(cube_vao_ci);

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
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	TextureBindingLayout *cubemapLayout = graphics_wrapper_->createTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = m_cubemap;
	stb.address = 4;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.layout = cubemapLayout;
	ci.textureCount = 1;
	m_cubemapBinding = m_graphics_wrapper_->createTextureBinding(ci);*/

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/irradiance.opengl.vert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/irradiance.opengl.frag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };

	sh_buffer.sh0[0] = glm::vec3(2.58676, 2.730808, 3.152812);
	sh_buffer.sh0[1] = glm::vec3(-0.431493, -0.665128, -0.969124);
	sh_buffer.sh0[2] = glm::vec3(-0.353886, 0.048348, 0.672755);
	sh_buffer.sh0[3] = glm::vec3(-0.604269, -0.88623, -1.298684);
	sh_buffer.sh0[4] = glm::vec3(0.320121, 0.422942, 0.541783);
	sh_buffer.sh0[5] = glm::vec3(-0.137435, -0.168666, -0.229637);
	sh_buffer.sh0[6] = glm::vec3(-0.052101, -0.149999, -0.232127);
	sh_buffer.sh0[7] = glm::vec3(-0.117312, -0.167151, -0.265015);
	sh_buffer.sh0[8] = glm::vec3(-0.090028, -0.021071, 0.08956);
	/*sh_buffer.sh1[0] = glm::vec3(2.58676, 2.730808, 3.152812);
	sh_buffer.sh1[1] = glm::vec3(-0.431493, -0.665128, -0.969124);
	sh_buffer.sh1[2] = glm::vec3(-0.353886, 0.048348, 0.672755);
	sh_buffer.sh1[3] = glm::vec3(-0.604269, -0.88623, -1.298684);
	sh_buffer.sh1[4] = glm::vec3(0.320121, 0.422942, 0.541783);
	sh_buffer.sh1[5] = glm::vec3(-0.137435, -0.168666, -0.229637);
	sh_buffer.sh1[6] = glm::vec3(-0.052101, -0.149999, -0.232127);
	sh_buffer.sh1[7] = glm::vec3(-0.117312, -0.167151, -0.265015);
	sh_buffer.sh1[8] = glm::vec3(-0.090028, -0.021071, 0.08956);*/


	for (int i = 0; i < 9; i++) {
		sh_buffer.sh1[i].r = ((float)rand() / (RAND_MAX));
		sh_buffer.sh1[i].g = ((float)rand() / (RAND_MAX));
		sh_buffer.sh1[i].b = ((float)rand() / (RAND_MAX));
		sh_buffer.sh1[i].r *= sh_buffer.sh1[i].r;
		sh_buffer.sh1[i].g *= sh_buffer.sh1[i].g;
		sh_buffer.sh1[i].b *= sh_buffer.sh1[i].b;
	}

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "SphericalHarmonicsBuffer";
	ubbci.size = sizeof(sh_buffer);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	sh_ubb = engine.getGraphicsWrapper()->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo uboci;
	uboci.isDynamic = true;
	uboci.size = sizeof(sh_buffer);
	uboci.binding = sh_ubb;
	sh_ubo = graphics_wrapper->createUniformBuffer(uboci);
	sh_ubo->updateBuffer(&sh_buffer);
	
	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo iblGPCI;
	iblGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	iblGPCI.vertex_bindings = &cube_vertex_layout_;
	iblGPCI.vertex_bindings_count = 1;
	iblGPCI.width = (float)engine.getSettings()->resolution_x_;
	iblGPCI.height = (float)engine.getSettings()->resolution_y_;
	iblGPCI.scissorW = engine.getSettings()->resolution_x_;
	iblGPCI.scissorH = engine.getSettings()->resolution_y_;
	iblGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	iblGPCI.shaderStageCreateInfos = stages.data();
	iblGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout*> tbls_ = { engine.getGbufferTBL() };
	iblGPCI.textureBindings = tbls_.data();
	iblGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding*> ubb = { engine.deff_ubb_, sh_ubb };
	iblGPCI.uniformBufferBindings = ubb.data();
	iblGPCI.uniformBufferBindingCount = ubb.size();
	sh_pipeline = engine.getGraphicsWrapper()->createGraphicsPipeline(iblGPCI);
}

void RenderPathDeferred::recreateFramebuffer(unsigned int w, unsigned int h) {
	destroyFramebuffers();
	CreateFramebuffer(w, h);
}

void RenderPathDeferred::destroyFramebuffers() {
	auto gw = engine.getGraphicsWrapper();

	if (render_targets_) {
		gw->deleteRenderTarget(render_targets_);
		render_targets_ = nullptr;
	}
	if (depth_target_) {
		gw->deleteDepthTarget(depth_target_);
		depth_target_ = nullptr;
	}
	if (gbuffer_) {
		gw->deleteFramebuffer(gbuffer_);
		gbuffer_ = nullptr;
	}
}

void RenderPathDeferred::destroyGraphics() {
	auto gw = engine.getGraphicsWrapper();

	if (debug_pipeline_) {
		gw->deleteGraphicsPipeline(debug_pipeline_);
		debug_pipeline_ = nullptr;
	}
	if (point_light_pipeline_) {
		gw->deleteGraphicsPipeline(point_light_pipeline_);
		point_light_pipeline_ = nullptr;
	}
	if (directional_light_pipeline_) {
		gw->deleteGraphicsPipeline(directional_light_pipeline_);
		directional_light_pipeline_ = nullptr;
	}
	if (spot_light_pipeline_) {
		gw->deleteGraphicsPipeline(spot_light_pipeline_);
		spot_light_pipeline_ = nullptr;
	}

	if (shadow_tbl_) {
		gw->deleteTextureBindingLayout(shadow_tbl_);
		shadow_tbl_ = nullptr;
	}

	if (point_light_ubb_) {
		gw->deleteUniformBufferBinding(point_light_ubb_);
		point_light_ubb_ = nullptr;
	}

	if (spot_light_ubb_) {
		gw->deleteUniformBufferBinding(spot_light_ubb_);
		spot_light_ubb_ = nullptr;
	}

	if (directional_light_ubb_) {
		gw->deleteUniformBufferBinding(directional_light_ubb_);
		directional_light_ubb_ = nullptr;
	}

	if (debug_ubb_) {
		gw->deleteUniformBufferBinding(debug_ubb_);
		debug_ubb_ = nullptr;
	}

	if (point_light_ubo_handler_) {
		gw->deleteUniformBuffer(point_light_ubo_handler_);
		point_light_ubo_handler_ = nullptr;
	}
	if (spot_light_ubo_handler_) {
		gw->deleteUniformBuffer(spot_light_ubo_handler_);
		spot_light_ubo_handler_ = nullptr;
	}

	if (directional_light_ubo_handler_) {
		gw->deleteUniformBuffer(directional_light_ubo_handler_);
		directional_light_ubo_handler_ = nullptr;
	}

	if (debug_ubo_handler_) {
		gw->deleteUniformBuffer(debug_ubo_handler_);
		debug_ubo_handler_ = nullptr;
	}
	destroyFramebuffers();
}

void RenderPathDeferred::reloadGraphics() {
	createShadowTextureBindingLayout();
	createPointLightShader();
	createSpotLightShader();
	createDirectionalLightShader();
	createDebugShader();
}

void RenderPathDeferred::setDebugMode(unsigned int d) {
	if (d >= 0 && d <= 7) {
		debug_mode_ = d;
	}
}

unsigned int RenderPathDeferred::getDebugMode() {
	return debug_mode_;
}

void RenderPathDeferred::createDebugShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "Debug";
	ubbci.size = sizeof(debug_mode_);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	debug_ubb_ = engine.getGraphicsWrapper()->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo uboci;
	uboci.isDynamic = true;
	uboci.size = sizeof(debug_mode_);
	uboci.binding = debug_ubb_;
	debug_ubo_handler_ = graphics_wrapper->createUniformBuffer(uboci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.glsl";
		fi.fileName = "../assets/shaders/debug/debug.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GraphicsLanguage::DirectX) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		fi.fileName = "../assets/shaders/debug/debug.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.spv";
		fi.fileName = "../assets/shaders/debug/debug.spv";
	}
	std::vector<char> vfile;
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo gpci;
	gpci.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	gpci.vertex_bindings = &vertex_layout;
	gpci.vertex_bindings_count = 1;
	gpci.width = (float)engine.getSettings()->resolution_x_;
	gpci.height = (float)engine.getSettings()->resolution_y_;
	gpci.scissorW = engine.getSettings()->resolution_x_;
	gpci.scissorH = engine.getSettings()->resolution_y_;
	gpci.primitiveType = Grindstone::GraphicsAPI::GeometryType::TriangleStrips;
	gpci.shaderStageCreateInfos = stages.data();
	gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout *> tbls_ = { engine.getGbufferTBL() };
	gpci.textureBindings = tbls_.data();
	gpci.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs = { engine.deff_ubb_, debug_ubb_ };
	gpci.uniformBufferBindings = ubbs.data();
	gpci.uniformBufferBindingCount = (uint32_t)ubbs.size();
	debug_pipeline_ = engine.getGraphicsWrapper()->createGraphicsPipeline(gpci);
}

void RenderPathDeferred::createPointLightShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo light_ubbci;
	light_ubbci.binding = 1;
	light_ubbci.shaderLocation = "Light";
	light_ubbci.size = sizeof(LightPointUBO);
	light_ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	point_light_ubb_ = engine.getGraphicsWrapper()->createUniformBufferBinding(light_ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = true;
	lightuboci.size = sizeof(LightPointUBO);
	lightuboci.binding = point_light_ubb_;
	point_light_ubo_handler_ = graphics_wrapper->createUniformBuffer(lightuboci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/pointVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/pointFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo pointGPCI;
	pointGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	pointGPCI.vertex_bindings = &vertex_layout;
	pointGPCI.vertex_bindings_count = 1;
	pointGPCI.width = (float)engine.getSettings()->resolution_x_;
	pointGPCI.height = (float)engine.getSettings()->resolution_y_;
	pointGPCI.scissorW = engine.getSettings()->resolution_x_;
	pointGPCI.scissorH = engine.getSettings()->resolution_y_;
	pointGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::TriangleStrips;
	pointGPCI.shaderStageCreateInfos = stages.data();
	pointGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout *> tbls_ = { engine.getGbufferTBL(), shadow_tbl_ };
	pointGPCI.textureBindings = tbls_.data();
	pointGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs = { engine.deff_ubb_, point_light_ubb_ };
	pointGPCI.uniformBufferBindings = ubbs.data();
	pointGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	point_light_pipeline_ = engine.getGraphicsWrapper()->createGraphicsPipeline(pointGPCI);
}

void RenderPathDeferred::createSpotLightShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo light_ubbci;
	light_ubbci.binding = 1;
	light_ubbci.shaderLocation = "Light";
	light_ubbci.size = sizeof(LightSpotUBO);
	light_ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	spot_light_ubb_ = engine.getGraphicsWrapper()->createUniformBufferBinding(light_ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = true;
	lightuboci.size = sizeof(LightSpotUBO);
	lightuboci.binding = spot_light_ubb_;
	spot_light_ubo_handler_ = graphics_wrapper->createUniformBuffer(lightuboci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/spotFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo spotGPCI;
	spotGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	spotGPCI.vertex_bindings = &vertex_layout;
	spotGPCI.vertex_bindings_count = 1;
	spotGPCI.width = (float)engine.getSettings()->resolution_x_;
	spotGPCI.height = (float)engine.getSettings()->resolution_y_;
	spotGPCI.scissorW = engine.getSettings()->resolution_x_;
	spotGPCI.scissorH = engine.getSettings()->resolution_y_;
	spotGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::TriangleStrips;
	spotGPCI.shaderStageCreateInfos = stages.data();
	spotGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout *> tbls_ = { engine.getGbufferTBL(), shadow_tbl_ };
	spotGPCI.textureBindings = tbls_.data();
	spotGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs = { engine.deff_ubb_, spot_light_ubb_ };
	spotGPCI.uniformBufferBindings = ubbs.data();
	spotGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	spot_light_pipeline_ = engine.getGraphicsWrapper()->createGraphicsPipeline(spotGPCI);
}

void RenderPathDeferred::createDirectionalLightShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo light_ubbci;
	light_ubbci.binding = 1;
	light_ubbci.shaderLocation = "Light";
	light_ubbci.size = sizeof(LightDirectionalUBO);
	light_ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	directional_light_ubb_ = engine.getGraphicsWrapper()->createUniformBufferBinding(light_ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo lightuboci;
	lightuboci.isDynamic = true;
	lightuboci.size = sizeof(LightDirectionalUBO);
	lightuboci.binding = directional_light_ubb_;
	directional_light_ubo_handler_ = graphics_wrapper->createUniformBuffer(lightuboci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		fi.fileName = "../assets/shaders/lights_deferred/directionalFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	std::vector<Grindstone::GraphicsAPI::ShaderStageCreateInfo> stages = { vi, fi };

	auto vertex_layout = engine.getPlaneVertexLayout();

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo directionalGPCI;
	directionalGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	directionalGPCI.vertex_bindings = &vertex_layout;
	directionalGPCI.vertex_bindings_count = 1;
	directionalGPCI.width = (float)engine.getSettings()->resolution_x_;
	directionalGPCI.height = (float)engine.getSettings()->resolution_y_;
	directionalGPCI.scissorW = engine.getSettings()->resolution_x_;
	directionalGPCI.scissorH = engine.getSettings()->resolution_y_;
	directionalGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::TriangleStrips;
	directionalGPCI.shaderStageCreateInfos = stages.data();
	directionalGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<Grindstone::GraphicsAPI::TextureBindingLayout *> tbls_ = { engine.getGbufferTBL(), shadow_tbl_ };
	directionalGPCI.textureBindings = tbls_.data();
	directionalGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs = { engine.deff_ubb_, directional_light_ubb_ };
	directionalGPCI.uniformBufferBindings = ubbs.data();
	directionalGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	directional_light_pipeline_ = engine.getGraphicsWrapper()->createGraphicsPipeline(directionalGPCI);
}

void RenderPathDeferred::render(Grindstone::GraphicsAPI::Framebuffer *fbo, Grindstone::GraphicsAPI::DepthTarget *depthTarget, Space *space) {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto pipeline = engine.getGraphicsPipelineManager();

	// Set fbo to gbuffer
	gbuffer_->Bind(true);

	// Clear screen
	gbuffer_->Clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);

	// Set as Opaque
	graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);

	// Render opaque elements
	/*if (wireframe_) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}*/

	pipeline->drawDeferredImmediate();

	/*if (wireframe_) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}*/

	// Prepare deffered stage information
	engine.deff_ubo_handler_->Bind();
	if (debug_mode_ != 0) {
		renderDebug(fbo);
	}
	else
	{
		// Deferred
		renderLights(fbo, space);
		
		/*gbuffer_->BindRead();
		engine.hdr_framebuffer_->BindWrite(true);
		m_graphics_wrapper_->copyToDepthBuffer(engine.depth_image_);
		engine.hdr_framebuffer_->BindRead();

		m_graphics_wrapper_->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
		// Unlit
		engine.materialManager.DrawUnlitImmediate();	
		// Forward
		m_graphics_wrapper_->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::AdditiveAlpha);
		engine.ubo->Bind();
		engine.ubo2->Bind();
		engine.materialManager.DrawForwardImmediate();
		m_graphics_wrapper_->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
		engine.deffUBO->Bind();
		engine.skybox_.Render();*/
		
		/*graphics_wrapper_->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
		pipeline_ssr_->Bind();
		hdr_framebuffer_->Bind(false);
		hdr_framebuffer_->BindTextures(4);
		deffUBO->Bind();
		gbuffer_->BindRead();
		gbuffer_->BindTextures(0);
		graphics_wrapper_->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);

		//exposure_buffer_.exposure = hdr_framebuffer_->getExposure(0);
		//exposure_buffer_.exposure = exposure_buffer_.exposure*100.0f/12.5f;
		//std::cout << exposure_buffer_.exposure << "\n";
		//exposure_ub_->updateBuffer(&exposure_buffer_);
		
		/*graphics_wrapper_->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
		pipeline_bloom_->Bind();
		hdr_framebuffer_->Bind(false);
		hdr_framebuffer_->BindTextures(0);
		graphics_wrapper_->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);*/

		/*pipeline_tonemap_->Bind();
		exposure_ub_->Bind();
		graphics_wrapper_bindDefaultFramebuffer(false);
		graphics_wrapper_->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
		hdr_framebuffer_->BindRead();
		hdr_framebuffer_->BindTextures(4);
		graphics_wrapper_->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);*/

		//CCamera *cam = &engine.cameraSystem.components[0];
		//cam->PostProcessing();
	
#if 1
		if (fbo) {
			fbo->BindWrite(true);
			//fbo->Clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
		}
		else {
			graphics_wrapper->bindDefaultFramebuffer(true);
			//graphics_wrapper->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
		}

		if (wireframe_) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		
		gbuffer_->BindRead();
		engine.getGraphicsWrapper()->copyToDepthBuffer(depthTarget);
		graphics_wrapper->enableDepth(true);

		engine.getUniformBuffer()->Bind();

		pipeline->drawUnlitImmediate();
		graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::AdditiveAlpha);
		if (fbo)
			fbo->BindTextures(0);
		pipeline->drawForwardImmediate();
		graphics_wrapper->enableDepth(false);

		if (wireframe_) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		gbuffer_->BindRead();
		gbuffer_->BindTextures(0);

		engine.deff_ubo_handler_->Bind();
#endif
	}
}

void RenderPathDeferred::renderDebug(Grindstone::GraphicsAPI::Framebuffer* fbo) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	float col[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	if (fbo) {
		fbo->BindWrite(true);
		fbo->Clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
	}
	else {
		graphics_wrapper->bindDefaultFramebuffer(true);
		graphics_wrapper->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);
	}

	gbuffer_->BindRead();
	gbuffer_->BindTextures(0);

	debug_ubo_handler_->updateBuffer(&debug_mode_);
	debug_ubo_handler_->Bind();

	debug_pipeline_->Bind();
	graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());
	graphics_wrapper->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
}

void RenderPathDeferred::renderLights(Grindstone::GraphicsAPI::Framebuffer *fbo, Space *space) {
	if (!space) return;
	GRIND_PROFILE_FUNC();
	auto graphics_wrapper = engine.getGraphicsWrapper();
	float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	TransformSubSystem *transform_system = (TransformSubSystem *)space->getSubsystem(COMPONENT_TRANSFORM);

	if (fbo) {
		fbo->BindWrite(true);
		fbo->Clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth);
	}
	else {
		graphics_wrapper->bindDefaultFramebuffer(true);
		graphics_wrapper->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);
	}

	graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::None);
	gbuffer_->BindRead();
	gbuffer_->BindTextures(0);

	/*sh_pipeline->Bind();
	sh_buffer.pvm = glm::mat4(1);
	sh_ubo->updateBuffer(&sh_buffer);
	sh_ubo->Bind();
	graphics_wrapper->bindVertexArrayObject(cube_vao_);
	graphics_wrapper->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::Triangles, true, 0, 0, cube_indices.size());*/

	graphics_wrapper->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::Additive);
	LightPointSubSystem *point_light_system = (LightPointSubSystem *)space->getSubsystem(COMPONENT_LIGHT_POINT);
	if (point_light_system && point_light_system->getNumComponents() > 0) {
		point_light_pipeline_->Bind();
		for (size_t i = 0; i < point_light_system->getNumComponents(); ++i) {
			auto &light = point_light_system->getComponent((ComponentHandle)i);
			GameObjectHandle game_object_handle = light.game_object_handle_;
			ComponentHandle component_transform_handle = space->getObject(game_object_handle).getComponentHandle(COMPONENT_TRANSFORM);

			light_point_ubo_.position = transform_system->getPosition(component_transform_handle);
			light_point_ubo_.color = light.properties_.color;
			light_point_ubo_.attenuationRadius = light.properties_.attenuationRadius;
			light_point_ubo_.power = light.properties_.power;
			light_point_ubo_.shadow = light.properties_.shadow;
			point_light_ubo_handler_->updateBuffer(&light_point_ubo_);

			point_light_ubo_handler_->Bind();

			graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());
			graphics_wrapper->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
		}
	}

	LightSpotSubSystem *spot_light_system = (LightSpotSubSystem *)space->getSubsystem(COMPONENT_LIGHT_SPOT);
	if (spot_light_pipeline_ && spot_light_system->getNumComponents() > 0) {
		spot_light_pipeline_->Bind();
		for (size_t i = 0; i < spot_light_system->getNumComponents(); ++i) {
			auto &light = spot_light_system->getComponent((ComponentHandle)i);
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
			spot_light_ubo_handler_->updateBuffer(&light_spot_ubo_);

			if (light.properties_.shadow && light.shadow_fbo_) {
				light.shadow_fbo_->BindRead();
				light.shadow_fbo_->BindTextures(4);
			}

			spot_light_ubo_handler_->Bind();

			graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());
			graphics_wrapper->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
		}
	}

	LightDirectionalSubSystem *directional_light_system = (LightDirectionalSubSystem *)space->getSubsystem(COMPONENT_LIGHT_DIRECTIONAL);
	if (directional_light_system && directional_light_system->getNumComponents() > 0) {
		directional_light_pipeline_->Bind();
		for (size_t i = 0; i < directional_light_system->getNumComponents(); ++i) {
			auto &light = directional_light_system->getComponent((ComponentHandle)i);
			GameObjectHandle game_object_handle = light.game_object_handle_;
			ComponentHandle component_transform_handle = space->getObject(game_object_handle).getComponentHandle(COMPONENT_TRANSFORM);

			light_directional_ubo_.direction = transform_system->getForward(component_transform_handle);
			light_directional_ubo_.color = light.properties_.color;
			light_directional_ubo_.source_radius = light.properties_.sourceRadius;
			light_directional_ubo_.power = light.properties_.power;
			light_directional_ubo_.shadow = light.properties_.shadow;
			light_directional_ubo_.shadow_mat = bias_matrix * light.shadow_mat_;
			light_directional_ubo_.shadow_resolution = light.properties_.resolution;
			directional_light_ubo_handler_->updateBuffer(&light_directional_ubo_);

			if (light.properties_.shadow && light.shadow_fbo_) {
				light.shadow_fbo_->BindRead();
				light.shadow_fbo_->BindTextures(4);
			}

			directional_light_ubo_handler_->Bind();

			graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());
			graphics_wrapper->drawImmediateVertices(Grindstone::GraphicsAPI::GeometryType::Triangles, 0, 6);
		}
	}
	
}

void RenderPathDeferred::CreateFramebuffer(unsigned int width, unsigned int height) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	std::vector<Grindstone::GraphicsAPI::RenderTargetCreateInfo> gbuffer_images_ci;
	gbuffer_images_ci.reserve(3);
	gbuffer_images_ci.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, width, height); // R  G  B matID
	gbuffer_images_ci.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R16G16B16A16, width, height); // nX nY nZ
	gbuffer_images_ci.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, width, height); // sR sG sB Roughness
	render_targets_ = graphics_wrapper->createRenderTarget(gbuffer_images_ci.data(), (uint32_t)gbuffer_images_ci.size());

	Grindstone::GraphicsAPI::DepthTargetCreateInfo depth_image_ci(Grindstone::GraphicsAPI::DepthFormat::D24_STENCIL_8, width, height, false, false);
	depth_target_ = graphics_wrapper->createDepthTarget(depth_image_ci);

	Grindstone::GraphicsAPI::FramebufferCreateInfo gbuffer_ci;
	gbuffer_ci.render_target_lists = &render_targets_;
	gbuffer_ci.num_render_target_lists = 1;
	gbuffer_ci.depth_target = depth_target_;
	gbuffer_ci.render_pass = nullptr;
	gbuffer_ = graphics_wrapper->createFramebuffer(gbuffer_ci);

}
