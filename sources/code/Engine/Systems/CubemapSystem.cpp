#include "CubemapSystem.hpp"
#include <iostream>

#include "../Core/Engine.hpp"

#include <fstream>

#include <cstring>

#include "../Utilities/Logger.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include "TransformSystem.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include <GraphicsWrapper.hpp>

#include "Core/Input.hpp"
#include "Core/Camera.hpp"
#include "Renderpaths/RenderPath.hpp"
#include "../Converter/ImageConverter.hpp"
#include <thread>

//#include <GL/gl3w.h>

///////////////////////////////////////////////////////////////////////////////
// find middle point of 2 vertices
// NOTE: new vertex must be resized, so the length is equal to the radius
///////////////////////////////////////////////////////////////////////////////
void computeHalfVertex(float radius, const float v1[3], const float v2[3], float newV[3])
{
	newV[0] = v1[0] + v2[0];    // x
	newV[1] = v1[1] + v2[1];    // y
	newV[2] = v1[2] + v2[2];    // z
	float scale = radius / sqrtf(newV[0] * newV[0] + newV[1] * newV[1] + newV[2] * newV[2]);
	newV[0] *= scale;
	newV[1] *= scale;
	newV[2] *= scale;
}

void CubemapSystem::prepareSphere() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	std::vector<glm::vec3> vertices = {
		glm::vec3(-0.525731f, 0, 0.850651f), glm::vec3(0.525731f, 0, 0.850651f),
		glm::vec3(-0.525731f, 0, -0.850651f), glm::vec3(0.525731f, 0, -0.850651f),
		glm::vec3(0, 0.850651f, 0.525731f), glm::vec3(0, 0.850651f, -0.525731f),
		glm::vec3(0, -0.850651f, 0.525731f), glm::vec3(0, -0.850651f, -0.525731f),
		glm::vec3(0.850651f, 0.525731f, 0), glm::vec3(-0.850651f, 0.525731f, 0),
		glm::vec3(0.850651f, -0.525731f, 0), glm::vec3(-0.850651f, -0.525731f, 0)
	};

	std::vector<uint32_t> indices = {
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	total_sphere_indices_ = (unsigned int)indices.size();

	sphere_vbd_.binding = 0;
	sphere_vbd_.elementRate = false;
	sphere_vbd_.stride = sizeof(glm::vec3);
	// sphere_vbd_.stride = sizeof(float) * 2; // CHANGE

	sphere_vad_.binding = 0;
	sphere_vad_.location = 0;
	//sphere_vad_.format = VERTEX_R32_G32;
	sphere_vad_.format = VERTEX_R32_G32_B32; // CHANGE
	sphere_vad_.size = 3;
	// sphere_vad_.size = 2; // CHANGE
	sphere_vad_.name = "vertexPosition";
	sphere_vad_.offset = 0;
	sphere_vad_.usage = ATTRIB_POSITION;

	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	VertexBufferCreateInfo sphere_vbo_ci;
	sphere_vbo_ci.binding = &sphere_vbd_;
	sphere_vbo_ci.bindingCount = 1;
	sphere_vbo_ci.attribute = &sphere_vad_;
	sphere_vbo_ci.attributeCount = 1;
	sphere_vbo_ci.content = vertices.data();
	sphere_vbo_ci.count = (uint32_t)vertices.size();
	sphere_vbo_ci.size = (uint32_t)(sizeof(glm::vec3) * vertices.size());


	VertexArrayObjectCreateInfo sphere_vao_ci;
	sphere_vao_ci.vertexBuffer = sphere_vbo_;
	sphere_vao_ci.indexBuffer = sphere_ibo_;
	sphere_vao_ = graphics_wrapper->CreateVertexArrayObject(sphere_vao_ci);
	sphere_vbo_ = graphics_wrapper->CreateVertexBuffer(sphere_vbo_ci);
	sphere_ibo_ = graphics_wrapper->CreateIndexBuffer(ibci);
	sphere_vao_ci.vertexBuffer = sphere_vbo_;
	sphere_vao_ci.indexBuffer = sphere_ibo_;
	sphere_vao_->BindResources(sphere_vao_ci);
	sphere_vao_->Unbind();
}

void CubemapSystem::prepareUniformBuffer() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "ConvolutionBufferObject";
	ubbci.size = sizeof(ConvolutionBufferObject);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	ubb_ = graphics_wrapper->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(ConvolutionBufferObject);
	ubci.binding = ubb_;
	ub_ = graphics_wrapper->CreateUniformBuffer(ubci);
}

void CubemapSystem::prepareIrradianceShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	// Create Irradiance
	int res = 512;
	std::vector<RenderTargetCreateInfo> irr_images_ci;
	irr_images_ci.reserve(1);
	irr_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, res, res);
	irradiance_image_ = graphics_wrapper->CreateRenderTarget(irr_images_ci.data(), (uint32_t)irr_images_ci.size());

	FramebufferCreateInfo irr_ci;
	irr_ci.render_target_lists = &irradiance_image_;
	irr_ci.num_render_target_lists = 1;
	irr_ci.depth_target = nullptr;
	irr_ci.render_pass = nullptr;
	irradiance_fbo_ = graphics_wrapper->CreateFramebuffer(irr_ci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.glsl";
		fi.fileName = "../assets/shaders/baking/cubemap_irr.glsl";
	}
	else if (settings->graphics_language_ == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.fxc";
		fi.fileName = "../assets/shaders/baking/cubemap_irr.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.spv";
		fi.fileName = "../assets/shaders/baking/cubemap_irr.spv";
	}

	vfile.clear();
	if (!readFile(vi.fileName, vfile)) {
		throw std::runtime_error("Irradiance Convolution Vertex Shader missing.\n");
		return;
	}
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("Irradiance Convolution Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	ShaderStageCreateInfo stages[2] = { vi, fi };

	GraphicsPipelineCreateInfo irrGPCI;
	irrGPCI.cullMode = CULL_BACK;
	irrGPCI.bindings = &sphere_vbd_;
	irrGPCI.bindingsCount = 1;
	irrGPCI.attributes = &sphere_vad_;
	irrGPCI.attributesCount = 1;
	irrGPCI.width = (float)res;
	irrGPCI.height = (float)res;
	irrGPCI.scissorW = res;
	irrGPCI.scissorH = res;
	irrGPCI.primitiveType = PRIM_TRIANGLES;
	irrGPCI.shaderStageCreateInfos = stages;
	irrGPCI.shaderStageCreateInfoCount = 2;
	irrGPCI.textureBindings = &texture_binding_layout_;
	irrGPCI.textureBindingCount = 1;
	irrGPCI.uniformBufferBindings = &ubb_;
	irrGPCI.uniformBufferBindingCount = 1;
	irradiance_pipeline_ = graphics_wrapper->CreateGraphicsPipeline(irrGPCI);
}

void CubemapSystem::prepareSpecularShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	// Create Specular
	int res = 512;
	std::vector<RenderTargetCreateInfo> spec_images_ci;
	spec_images_ci.reserve(1);
	spec_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, res, res);
	specular_image_ = graphics_wrapper->CreateRenderTarget(spec_images_ci.data(), (uint32_t)spec_images_ci.size());

	FramebufferCreateInfo spec_ci;
	spec_ci.render_target_lists = &specular_image_;
	spec_ci.num_render_target_lists = 1;
	spec_ci.depth_target = nullptr;
	spec_ci.render_pass = nullptr;
	specular_fbo_ = graphics_wrapper->CreateFramebuffer(spec_ci);

	// Create Pipeline
	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.glsl";
		fi.fileName = "../assets/shaders/baking/cubemap_spec.glsl";
	}
	else if (settings->graphics_language_ == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.fxc";
		fi.fileName = "../assets/shaders/baking/cubemap_spec.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.spv";
		fi.fileName = "../assets/shaders/baking/cubemap_spec.spv";
	}

	vfile.clear();
	if (!readFile(vi.fileName, vfile)) {
		throw std::runtime_error("Specular Convolution Vertex Shader missing.\n");
		return;
	}
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("Specular Convolution Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	ShaderStageCreateInfo stages[2] = { vi, fi };

	GraphicsPipelineCreateInfo specGPCI;
	specGPCI.cullMode = CULL_BACK;
	specGPCI.bindings = &sphere_vbd_;
	specGPCI.bindingsCount = 1;
	specGPCI.attributes = &sphere_vad_;
	specGPCI.attributesCount = 1;
	specGPCI.width = (float)res;
	specGPCI.height = (float)res;
	specGPCI.scissorW = res;
	specGPCI.scissorH = res;
	specGPCI.primitiveType = PRIM_TRIANGLES;
	specGPCI.shaderStageCreateInfos = stages;
	specGPCI.shaderStageCreateInfoCount = 2;
	specGPCI.textureBindings = &texture_binding_layout_;
	specGPCI.textureBindingCount = 1;
	specGPCI.uniformBufferBindings = &ubb_;
	specGPCI.uniformBufferBindingCount = 1;
	specular_pipeline_ = graphics_wrapper->CreateGraphicsPipeline(specGPCI);
}

void CubemapSystem::convoluteIrradiance(CubemapComponent &c) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	graphics_wrapper->BindVertexArrayObject(sphere_vao_);
	//graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());

	// graphics_wrapper->EnableDepth(false);
	// Bind Irradiance Convolution graphics pipeline
	irradiance_pipeline_->Bind();

	// Bind Irradiance Convolution Framebuffer
	// irradiance_fbo_->BindWrite(false);

	// Bind camera framebuffer for read
	// camera_->final_framebuffer_->BindRead();
	// camera_->final_framebuffer_->BindTextures(0);

	int res = 512;
	unsigned char ***data = new unsigned char**[6];

	// For every face...
	for (int i = 0; i < 6; ++i) {
		// camera_->final_framebuffer_->BindRead();
		// Create view matrix for face
		glm::mat4 view = glm::lookAt(glm::vec3(0), gCubeDirections[i].Target, gCubeDirections[i].Up);
		ubo_.matrix_ = projection_ * view;
		ub_->UpdateUniformBuffer(&ubo_);
		ub_->Bind();

		// Allocate data
		data[i] = new unsigned char *();
		data[i][0] = new unsigned char[res * res * 4];


		irradiance_fbo_->BindWrite(true);

		camera_framebuffer_->BindRead();
		camera_framebuffer_->BindTextures(4);

		graphics_wrapper->setViewport(0, 0, res, res);

		// Clear it
		graphics_wrapper->Clear(CLEAR_BOTH);

		// Render
		graphics_wrapper->DrawImmediateIndexed(GEOMETRY_TRIANGLES, true, 0, 0, total_sphere_indices_);
		
		graphics_wrapper->SwapBuffer();
		irradiance_fbo_->BindRead();
		irradiance_image_->RenderScreen(0, res, res, data[i][0]);
	}

	// Export DDS
	std::string path = std::string("../assets/cubemaps/") + c.path_ + "_irr.dds";
	ConvertBC123(data, true, res, res, C_BC1, path, true);
	std::cout << "Outputting " << path << "\n";


	// graphics_wrapper->EnableDepth(true);
}

void CubemapSystem::convoluteSpecular(CubemapComponent &c) {
	return;
	auto graphics_wrapper = engine.getGraphicsWrapper();

	graphics_wrapper->BindVertexArrayObject(sphere_vao_);
	//graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());

	// graphics_wrapper->EnableDepth(false);
	// Bind Irradiance Convolution graphics pipeline
	specular_pipeline_->Bind();

	// Bind Irradiance Convolution Framebuffer
	// irradiance_fbo_->BindWrite(false);

	// Bind camera framebuffer for read
	// camera_->final_framebuffer_->BindRead();
	// camera_->final_framebuffer_->BindTextures(0);

	int res = 512; // c.resolution_;
	int mips = int(log(res)) + 1;
	unsigned char ***data = new unsigned char**[6];
	int mipres;

	// For every face...
	for (int i = 0; i < 6; ++i) {
		data[i] = new unsigned char *[mips];

		// Create view matrix for face
		glm::mat4 view = glm::lookAt(glm::vec3(0), gCubeDirections[i].Target, gCubeDirections[i].Up);
		ubo_.matrix_ = projection_ * view;

		mipres = res;

		// For every mip
		for (int m = 0; m < mips; ++m) {
			ubo_.roughness_ = (float)m / (float)(mips - 1);
			ub_->UpdateUniformBuffer(&ubo_);
			ub_->Bind();

			// Allocate data array
			data[i][m] = new unsigned char[mipres * mipres * 4];

			specular_fbo_->BindWrite(true);

			camera_framebuffer_->BindRead();
			camera_framebuffer_->BindTextures(4);

			// Clear it
			graphics_wrapper->Clear(CLEAR_BOTH);

			// Render
			graphics_wrapper->DrawImmediateIndexed(GEOMETRY_TRIANGLES, true, 0, 0, total_sphere_indices_);
			
			graphics_wrapper->SwapBuffer();
			specular_fbo_->BindRead();
			specular_image_->RenderScreen(0, mipres, mipres, data[i][m]);

			mipres /= 2;
		}
	}

	// Export DDS
	std::string path = std::string("../assets/cubemaps/") + c.path_ + "_spec.png";
	//ConvertBC123(data, true, res, res, C_BC1, path, false);
	engine.getTextureManager()->writeCubemap(path, data, 512);
	std::cout << "Outputting " << path << "\n";
}

void CubemapSystem::bake() {
	// For each cubemap component...
	for (auto scene : engine.getScenes()) {
		for (auto space : scene->spaces_) {
			TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));
			CubemapSubSystem *sub = (CubemapSubSystem *)space->getSubsystem(COMPONENT_CUBEMAP);
			for (auto &component : sub->components_) {
				// Copy Projection data.
				sub->camera_->near_ = component.near_;
				sub->camera_->far_ = component.far_;

				GameObjectHandle game_object_id = component.game_object_handle_;
				GameObject &obj = space->getObject(game_object_id);
				ComponentHandle transform_id = obj.getComponentHandle(COMPONENT_TRANSFORM);

				glm::vec3 pos = transform->getPosition(transform_id);

				// For every face...
				for (uint8_t i = 0; i < 6; ++i) {
					// Get view matrix for face
					//engine.getGraphicsWrapper()->setViewport(0, 0, component.resolution_, component.resolution_);

					// Render 10 times (to auto-adjust exposure)
					for (int j = 0; j < 10; ++j) {
						sub->camera_->setPosition(pos);
						sub->camera_->setDirections(gCubeDirections[i].Target, gCubeDirections[i].Up);
						sub->camera_->render();
					}
				}

				// Convolute and Export Irradiance and Specular
				convoluteIrradiance(component);
				convoluteSpecular(component);
			}
		}
	}
}

CubemapComponent::CubemapComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_CUBEMAP, object_handle, id), capture_method_(CubemapComponent::CaptureMethod::CAPTURE_BAKE), near_(0.1f), far_(100.0f), resolution_(512), cubemap_(nullptr), cubemap_binding_(nullptr), capture_fbo_(nullptr), render_target_(nullptr) {}

CubemapSystem::CubemapSystem() : System(COMPONENT_CUBEMAP) {
	auto gw = engine.getGraphicsWrapper();
	auto input = engine.getInputManager();

	cube_binding_ = TextureSubBinding("environmentMap", 4);

	projection_ = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 5.0f);

	loadGraphics();
}

CubemapSubSystem::CubemapSubSystem(Space *space) : SubSystem(COMPONENT_CUBEMAP, space) {

	// Prepare Camera
	camera_ = new Camera(space, true);
	camera_->setCustomFinalFramebuffer(engine.getSystem<CubemapSystem>()->camera_framebuffer_);
	camera_->projection_fov_ = glm::radians(90.0f);
	camera_->enable_reflections_ = true;
	camera_->enable_auto_exposure_ = false;
	camera_->setViewport(512, 512); // Un-hardcode this
	camera_->initialize();
}

void CubemapSubSystem::initialize() {
	for (auto &component : components_) {
		auto objname = space_->getObject(component.game_object_handle_).getName();
		component.path_ = objname;

		// Load File
		TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component.path_ + ".dds");
		if (handle == size_t(-1)) {
			component.cubemap_ = nullptr;
			component.cubemap_binding_ = nullptr;
		}
		else {
			Texture *texture = engine.getTextureManager()->getTexture(handle);
			component.cubemap_ = texture;
			SingleTextureBind stb;
			stb.texture = component.cubemap_;
			stb.address = 4;

			TextureBindingCreateInfo ci;
			ci.textures = &stb;
			ci.layout = engine.getSystem<CubemapSystem>()->texture_binding_layout_;
			ci.textureCount = 1;
			component.cubemap_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
		}
	}
}

ComponentHandle CubemapSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

/*
void CubemapSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];
	auto object_handle = component.game_object_handle_;

	if (params.HasMember("type")) {
		std::string type = params["type"].GetString();

		if (type == "baked") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_BAKE;

			auto objname = space_->getObject(object_handle).getName();
			component.path_ = objname;

			// Load File
			TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component.path_ + ".dds");
			if (handle == size_t(-1)) {
				component.cubemap_ = nullptr;
				component.cubemap_binding_ = nullptr;
			}
			else {
				Texture *texture = engine.getTextureManager()->getTexture(handle);
				component.cubemap_ = texture;
				SingleTextureBind stb;
				stb.texture = component.cubemap_;
				stb.address = 4;

				TextureBindingCreateInfo ci;
				ci.textures = &stb;
				ci.layout = texture_binding_layout_;
				ci.textureCount = 1;
				component.cubemap_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
			}
		}
		else if (type == "realtime") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_REALTIME;
		}
		else if (type == "custom") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_CUSTOM;

			if (params.HasMember("path")) {
				component.path_ = params["path"].GetString();

				// Load File
				TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component.path_);
				Texture *texture = engine.getTextureManager()->getTexture(handle);
				component.cubemap_ = texture;

				SingleTextureBind stb;
				stb.texture = component.cubemap_;
				stb.address = 4;

				TextureBindingCreateInfo ci;
				ci.textures = &stb;
				ci.layout = texture_binding_layout_;
				ci.textureCount = 1;
				component.cubemap_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
			}
			else {
				GRIND_WARN("No path given.");
			}
		}
		else {
			GRIND_WARN("Invalid type.");
		}
	}

	if (component.capture_method_ == CubemapComponent::CaptureMethod::CAPTURE_BAKE ||
		component.capture_method_ == CubemapComponent::CaptureMethod::CAPTURE_REALTIME) {
		if (params.HasMember("resolution")) {
			component.resolution_ = params["resolution"].GetUint();
		}

		if (params.HasMember("far")) {
			component.far_ = params["far"].GetFloat();
		}

		if (params.HasMember("near")) {
			component.near_ = params["far"].GetFloat();
		}

		RenderTargetCreateInfo gbuffer_images_ci(FORMAT_COLOR_R8G8B8A8, component.resolution_, component.resolution_);
		component.render_target_ = engine.getGraphicsWrapper()->CreateRenderTarget(&gbuffer_images_ci, 1);

		DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24_STENCIL_8, component.resolution_, component.resolution_, false, false);
		DepthTarget *depth_target_ = engine.getGraphicsWrapper()->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo gbuffer_ci;
		gbuffer_ci.render_target_lists = &component.render_target_;
		gbuffer_ci.num_render_target_lists = 1;
		gbuffer_ci.depth_target = depth_target_;
		gbuffer_ci.render_pass = nullptr;
		component.capture_fbo_ = engine.getGraphicsWrapper()->CreateFramebuffer(gbuffer_ci);
	}
}
*/

void CubemapSystem::update(double dt) {
	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
			}
		}
	}
}

void CubemapSystem::loadGraphics() {
	auto gw = engine.getGraphicsWrapper();

	RenderTargetCreateInfo fbo_buffer_ci(FORMAT_COLOR_R8G8B8, 512, 512);
	final_buffer_ = gw->CreateRenderTarget(&fbo_buffer_ci, 1, true);

	FramebufferCreateInfo final_framebuffer_ci;
	final_framebuffer_ci.render_target_lists = &final_buffer_;
	final_framebuffer_ci.num_render_target_lists = 1;
	final_framebuffer_ci.depth_target = nullptr; // depth_image_;
	final_framebuffer_ci.render_pass = nullptr;
	camera_framebuffer_ = gw->CreateFramebuffer(final_framebuffer_ci);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &cube_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	texture_binding_layout_ = gw->CreateTextureBindingLayout(tblci);

	prepareSphere();
	prepareUniformBuffer();
	prepareIrradianceShader();
	prepareSpecularShader();

	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				auto objname = space->getObject(component.game_object_handle_).getName();
				component.path_ = objname;

				// Load File
				TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component.path_ + ".dds");
				if (handle == size_t(-1)) {
					component.cubemap_ = nullptr;
					component.cubemap_binding_ = nullptr;
				}
				else {
					Texture *texture = engine.getTextureManager()->getTexture(handle);
					component.cubemap_ = texture;
					SingleTextureBind stb;
					stb.texture = component.cubemap_;
					stb.address = 4;

					TextureBindingCreateInfo ci;
					ci.textures = &stb;
					ci.layout = engine.getSystem<CubemapSystem>()->texture_binding_layout_;
					ci.textureCount = 1;
					component.cubemap_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
				}
			}
		}
	}
}

void CubemapSystem::destroyGraphics() {
	auto gw = engine.getGraphicsWrapper();

	gw->DeleteGraphicsPipeline(irradiance_pipeline_);
	gw->DeleteRenderTarget(irradiance_image_);
	gw->DeleteFramebuffer(irradiance_fbo_);

	gw->DeleteGraphicsPipeline(specular_pipeline_);
	gw->DeleteRenderTarget(specular_image_);
	gw->DeleteFramebuffer(specular_fbo_);

	gw->DeleteVertexArrayObject(sphere_vao_);
	gw->DeleteVertexBuffer(sphere_vbo_);
	gw->DeleteIndexBuffer(sphere_ibo_);
	gw->DeleteRenderTarget(final_buffer_);

	gw->DeleteUniformBufferBinding(ubb_);
	gw->DeleteUniformBuffer(ub_);

	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				gw->DeleteTextureBinding(component.cubemap_binding_);
				gw->DeleteFramebuffer(component.capture_fbo_);
				gw->DeleteRenderTarget(component.render_target_);
			}
		}
	}
}

CubemapComponent & CubemapSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * CubemapSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t CubemapSubSystem::getNumComponents() {
	return components_.size();
}

void CubemapSubSystem::removeComponent(ComponentHandle handle) {
}

void CubemapSystem::loadCubemaps() {
}

CubemapComponent * CubemapSubSystem::getClosestCubemap(glm::vec3 eye) {
	float dist_max = INFINITY;
	CubemapComponent *max = nullptr;

	for (auto &component : components_) {
		GameObjectHandle game_object_id = component.game_object_handle_;
		ComponentHandle transform_id = space_->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
		TransformSubSystem *transform = (TransformSubSystem *)(space_->getSubsystem(COMPONENT_TRANSFORM));

		glm::vec3 c = transform->getPosition(transform_id);
		float dist = glm::distance(eye, c);
		if (dist < dist_max) {
			max = &component;
			dist_max = dist;
		}
	}

	return max;
}

CubemapSubSystem::~CubemapSubSystem() {
}


void handleMode(void *owner) {
	CubemapComponent *component = ((CubemapComponent *)owner);
	
	auto objname = engine.getScene(0)->spaces_[0]->getObject(component->game_object_handle_).getName();
	component->path_ = objname;

	// Load File
	TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component->path_ + ".dds");
	if (handle == size_t(-1)) {
		component->cubemap_ = nullptr;
		component->cubemap_binding_ = nullptr;
	}
	else {
		Texture *texture = engine.getTextureManager()->getTexture(handle);
		component->cubemap_ = texture;
		SingleTextureBind stb;
		stb.texture = component->cubemap_;
		stb.address = 4;

		TextureBindingCreateInfo ci;
		ci.textures = &stb;
		ci.layout = engine.getSystem<CubemapSystem>()->texture_binding_layout_;
		ci.textureCount = 1;
		component->cubemap_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
	}
}

REFLECT_STRUCT_BEGIN(CubemapComponent, CubemapSystem, COMPONENT_CUBEMAP)
REFLECT_STRUCT_MEMBER_D(capture_method_, "Capture Method", "capturemethod", reflect::Metadata::SaveSetAndView, handleMode)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
