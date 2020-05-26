#include "CubemapSystem.hpp"
#include <iostream>

#include "../Core/Engine.hpp"

#include <fstream>

#include <cstring>

#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include "TransformSystem.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>

#include "Core/Input.hpp"
#include "Core/Camera.hpp"
#include "Renderpaths/RenderPath.hpp"
#include "../Converter/ImageConverter.hpp"
#include <thread>

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

	sphere_vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
	});

	Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	Grindstone::GraphicsAPI::VertexBufferCreateInfo sphere_vbo_ci;
	sphere_vbo_ci.layout = &sphere_vertex_layout_;
	sphere_vbo_ci.content = vertices.data();
	sphere_vbo_ci.count = (uint32_t)vertices.size();
	sphere_vbo_ci.size = (uint32_t)(sizeof(glm::vec3) * vertices.size());


	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo sphere_vao_ci;
	sphere_vbo_ = graphics_wrapper->createVertexBuffer(sphere_vbo_ci);
	sphere_ibo_ = graphics_wrapper->createIndexBuffer(ibci);
	sphere_vao_ci.vertex_buffers = &sphere_vbo_;
	sphere_vao_ci.vertex_buffer_count = 1;
	sphere_vao_ci.index_buffer = sphere_ibo_;
	sphere_vao_ = graphics_wrapper->createVertexArrayObject(sphere_vao_ci);
}

void CubemapSystem::prepareUniformBuffer() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "ConvolutionBufferObject";
	ubbci.size = sizeof(ConvolutionBufferObject);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	ubb_ = graphics_wrapper->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(ConvolutionBufferObject);
	ubci.binding = ubb_;
	ub_ = graphics_wrapper->createUniformBuffer(ubci);
}

void CubemapSystem::prepareIrradianceShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	// Create Irradiance
	int res = 512;
	std::vector<Grindstone::GraphicsAPI::RenderTargetCreateInfo> irr_images_ci;
	irr_images_ci.reserve(1);
	irr_images_ci.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, res, res);
	irradiance_image_ = graphics_wrapper->createRenderTarget(irr_images_ci.data(), (uint32_t)irr_images_ci.size());

	Grindstone::GraphicsAPI::FramebufferCreateInfo irr_ci;
	irr_ci.render_target_lists = &irradiance_image_;
	irr_ci.num_render_target_lists = 1;
	irr_ci.depth_target = nullptr;
	irr_ci.render_pass = nullptr;
	irradiance_fbo_ = graphics_wrapper->createFramebuffer(irr_ci);

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.glsl";
		fi.fileName = "../assets/shaders/baking/cubemap_irr.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("Irradiance Convolution Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2] = { vi, fi };

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo irrGPCI;
	irrGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	irrGPCI.vertex_bindings = &sphere_vertex_layout_;
	irrGPCI.vertex_bindings_count = 1;
	irrGPCI.width = (float)res;
	irrGPCI.height = (float)res;
	irrGPCI.scissorW = res;
	irrGPCI.scissorH = res;
	irrGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	irrGPCI.shaderStageCreateInfos = stages;
	irrGPCI.shaderStageCreateInfoCount = 2;
	irrGPCI.textureBindings = &texture_binding_layout_;
	irrGPCI.textureBindingCount = 1;
	irrGPCI.uniformBufferBindings = &ubb_;
	irrGPCI.uniformBufferBindingCount = 1;
	irradiance_pipeline_ = graphics_wrapper->createGraphicsPipeline(irrGPCI);
}

void CubemapSystem::prepareSpecularShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	// Create Specular
	int res = 512;
	std::vector<Grindstone::GraphicsAPI::RenderTargetCreateInfo> spec_images_ci;
	spec_images_ci.reserve(1);
	spec_images_ci.emplace_back(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, res, res);
	specular_image_ = graphics_wrapper->createRenderTarget(spec_images_ci.data(), (uint32_t)spec_images_ci.size());

	Grindstone::GraphicsAPI::FramebufferCreateInfo spec_ci;
	spec_ci.render_target_lists = &specular_image_;
	spec_ci.num_render_target_lists = 1;
	spec_ci.depth_target = nullptr;
	spec_ci.render_pass = nullptr;
	specular_fbo_ = graphics_wrapper->createFramebuffer(spec_ci);

	// Create Pipeline
	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/baking/cubemap_vert.glsl";
		fi.fileName = "../assets/shaders/baking/cubemap_spec.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
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
	vi.type = Grindstone::GraphicsAPI::ShaderStage::Vertex;

	ffile.clear();
	if (!readFile(fi.fileName, ffile)) {
		throw std::runtime_error("Specular Convolution Fragment Shader missing.\n");
		return;
	}
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = Grindstone::GraphicsAPI::ShaderStage::Fragment;

	Grindstone::GraphicsAPI::ShaderStageCreateInfo stages[2] = { vi, fi };

	Grindstone::GraphicsAPI::GraphicsPipelineCreateInfo specGPCI;
	specGPCI.cullMode = Grindstone::GraphicsAPI::CullMode::Back;
	specGPCI.vertex_bindings = &sphere_vertex_layout_;
	specGPCI.vertex_bindings_count = 1;
	specGPCI.width = (float)res;
	specGPCI.height = (float)res;
	specGPCI.scissorW = res;
	specGPCI.scissorH = res;
	specGPCI.primitiveType = Grindstone::GraphicsAPI::GeometryType::Triangles;
	specGPCI.shaderStageCreateInfos = stages;
	specGPCI.shaderStageCreateInfoCount = 2;
	specGPCI.textureBindings = &texture_binding_layout_;
	specGPCI.textureBindingCount = 1;
	specGPCI.uniformBufferBindings = &ubb_;
	specGPCI.uniformBufferBindingCount = 1;
	specular_pipeline_ = graphics_wrapper->createGraphicsPipeline(specGPCI);
}

void CubemapSystem::convoluteIrradiance(CubemapComponent &c) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	graphics_wrapper->bindVertexArrayObject(sphere_vao_);
	//graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());

	// graphics_wrapper->enableDepth(false);
	// Bind Irradiance Convolution graphics pipeline
	irradiance_pipeline_->Bind();

	// Bind Irradiance Convolution Grindstone::GraphicsAPI::Framebuffer
	// irradiance_fbo_->BindWrite(false);

	// Bind camera Grindstone::GraphicsAPI::Framebuffer for read
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
		ub_->updateBuffer(&ubo_);
		ub_->Bind();

		// Allocate data
		data[i] = new unsigned char *();
		data[i][0] = new unsigned char[res * res * 4];


		irradiance_fbo_->BindWrite(true);

		camera_framebuffer_->BindRead();
		camera_framebuffer_->BindTextures(4);

		graphics_wrapper->setViewport(0, 0, res, res);

		// Clear it
		float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		graphics_wrapper->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);

		// Render
		graphics_wrapper->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::TriangleStrips, true, 0, 0, total_sphere_indices_);
		
		graphics_wrapper->swapBuffers();
		irradiance_fbo_->BindRead();
		irradiance_image_->RenderScreen(0, res, res, data[i][0]);
	}

	// Export DDS
	std::string path = std::string("../assets/cubemaps/") + c.path_ + "_irr.dds";
	ConvertBC123(data, true, res, res, C_BC1, path, true);
	std::cout << "Outputting " << path << "\n";


	// graphics_wrapper->enableDepth(true);
}

void CubemapSystem::convoluteSpecular(CubemapComponent &c) {
	return;
	auto graphics_wrapper = engine.getGraphicsWrapper();

	graphics_wrapper->bindVertexArrayObject(sphere_vao_);
	//graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());

	// graphics_wrapper->enableDepth(false);
	// Bind Irradiance Convolution graphics pipeline
	specular_pipeline_->Bind();

	// Bind Irradiance Convolution Grindstone::GraphicsAPI::Framebuffer
	// irradiance_fbo_->BindWrite(false);

	// Bind camera Grindstone::GraphicsAPI::Framebuffer for read
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
			ub_->updateBuffer(&ubo_);
			ub_->Bind();

			// Allocate data array
			data[i][m] = new unsigned char[mipres * mipres * 4];

			specular_fbo_->BindWrite(true);

			camera_framebuffer_->BindRead();
			camera_framebuffer_->BindTextures(4);

			// Clear it
			float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			graphics_wrapper->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, col, 1.0f, 0);

			// Render
			graphics_wrapper->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::TriangleStrips, true, 0, 0, total_sphere_indices_);
			
			graphics_wrapper->swapBuffers();
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
	for (auto space : engine.getSpaces()) {
		TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));
		CubemapSubSystem *sub = (CubemapSubSystem *)space->getSubsystem(COMPONENT_CUBEMAP);
		for (auto &component : sub->components_) {
			// Copy Projection data.
			sub->camera_->near_ = component.near_;
			sub->camera_->far_ = component.far_;

			GameObjectHandle game_object_id = component.game_object_handle_;
			GameObject &obj = space->getObject(game_object_id);
			TransformComponent *transform = obj.getComponent<TransformComponent>();

			glm::vec3 pos = transform->getPosition();

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

CubemapComponent::CubemapComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_CUBEMAP, object_handle, id), capture_method_(CubemapComponent::CaptureMethod::CAPTURE_BAKE), near_(0.1f), far_(100.0f), resolution_(512), cubemap_(nullptr), cubemap_binding_(nullptr), capture_fbo_(nullptr), render_target_(nullptr) {}

CubemapSystem::CubemapSystem() : System(COMPONENT_CUBEMAP) {
	GRIND_PROFILE_FUNC();
	auto gw = engine.getGraphicsWrapper();
	auto input = engine.getInputManager();

	cube_binding_ = Grindstone::GraphicsAPI::TextureSubBinding("environmentMap", 4);

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
			Grindstone::GraphicsAPI::Texture *texture = engine.getTextureManager()->getTexture(handle);
			component.cubemap_ = texture;
			Grindstone::GraphicsAPI::SingleTextureBind stb;
			stb.texture = component.cubemap_;
			stb.address = 4;

			Grindstone::GraphicsAPI::TextureBindingCreateInfo ci;
			ci.textures = &stb;
			ci.layout = engine.getSystem<CubemapSystem>()->texture_binding_layout_;
			ci.textureCount = 1;
			component.cubemap_binding_ = engine.getGraphicsWrapper()->createTextureBinding(ci);
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
				component.cubemap_binding_ = engine.getGraphicsWrapper()->createTextureBinding(ci);
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
				component.cubemap_binding_ = engine.getGraphicsWrapper()->createTextureBinding(ci);
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

		RenderTargetCreateInfo gbuffer_images_ci(Grindstone::GraphicsAPI::ColorFormat::R8G8B8A8, component.resolution_, component.resolution_);
		component.render_target_ = engine.getGraphicsWrapper()->createRenderTarget(&gbuffer_images_ci, 1);

		DepthTargetCreateInfo depth_image_ci(Grindstone::GraphicsAPI::DepthFormat::D24_STENCIL_8, component.resolution_, component.resolution_, false, false);
		DepthTarget *depth_target_ = engine.getGraphicsWrapper()->createDepthTarget(depth_image_ci);

		Grindstone::GraphicsAPI::FramebufferCreateInfo gbuffer_ci;
		gbuffer_ci.render_target_lists = &component.render_target_;
		gbuffer_ci.num_render_target_lists = 1;
		gbuffer_ci.depth_target = depth_target_;
		gbuffer_ci.render_pass = nullptr;
		component.capture_fbo_ = engine.getGraphicsWrapper()->createFramebuffer(gbuffer_ci);
	}
}
*/

void CubemapSystem::update() {
	GRIND_PROFILE_FUNC();
	for (auto space : engine.getSpaces()) {
		CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
		for (auto &component : subsystem->components_) {
		}
	}
}

void CubemapSystem::loadGraphics() {
	auto gw = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::RenderTargetCreateInfo fbo_buffer_ci(Grindstone::GraphicsAPI::ColorFormat::R8G8B8, 512, 512);
	final_buffer_ = gw->createRenderTarget(&fbo_buffer_ci, 1, true);

	Grindstone::GraphicsAPI::FramebufferCreateInfo final_framebuffer_ci;
	final_framebuffer_ci.render_target_lists = &final_buffer_;
	final_framebuffer_ci.num_render_target_lists = 1;
	final_framebuffer_ci.depth_target = nullptr; // depth_image_;
	final_framebuffer_ci.render_pass = nullptr;
	camera_framebuffer_ = gw->createFramebuffer(final_framebuffer_ci);

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &cube_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	texture_binding_layout_ = gw->createTextureBindingLayout(tblci);

	prepareSphere();
	prepareUniformBuffer();
	prepareIrradianceShader();
	prepareSpecularShader();

	for (auto space : engine.getSpaces()) {
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
				Grindstone::GraphicsAPI::Texture *texture = engine.getTextureManager()->getTexture(handle);
				component.cubemap_ = texture;
				Grindstone::GraphicsAPI::SingleTextureBind stb;
				stb.texture = component.cubemap_;
				stb.address = 4;

				Grindstone::GraphicsAPI::TextureBindingCreateInfo ci;
				ci.textures = &stb;
				ci.layout = engine.getSystem<CubemapSystem>()->texture_binding_layout_;
				ci.textureCount = 1;
				component.cubemap_binding_ = engine.getGraphicsWrapper()->createTextureBinding(ci);
			}
		}
	}
}

void CubemapSystem::destroyGraphics() {
	auto gw = engine.getGraphicsWrapper();

	gw->deleteGraphicsPipeline(irradiance_pipeline_);
	gw->deleteRenderTarget(irradiance_image_);
	gw->deleteFramebuffer(irradiance_fbo_);

	gw->deleteGraphicsPipeline(specular_pipeline_);
	gw->deleteRenderTarget(specular_image_);
	gw->deleteFramebuffer(specular_fbo_);

	gw->deleteVertexArrayObject(sphere_vao_);
	gw->deleteVertexBuffer(sphere_vbo_);
	gw->deleteIndexBuffer(sphere_ibo_);
	gw->deleteRenderTarget(final_buffer_);

	gw->deleteUniformBufferBinding(ubb_);
	gw->deleteUniformBuffer(ub_);

	for (auto space : engine.getSpaces()) {
		CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
		for (auto &component : subsystem->components_) {
			gw->deleteTextureBinding(component.cubemap_binding_);
			gw->deleteFramebuffer(component.capture_fbo_);
			gw->deleteRenderTarget(component.render_target_);
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
		TransformComponent *transform = space_->getObject(game_object_id).getComponent<TransformComponent>();
		
		glm::vec3 c = transform->getPosition();
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
	
	auto objname = engine.getSpace(0)->getObject(component->game_object_handle_).getName();
	component->path_ = objname;

	// Load File
	TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component->path_ + ".dds");
	if (handle == size_t(-1)) {
		component->cubemap_ = nullptr;
		component->cubemap_binding_ = nullptr;
	}
	else {
		Grindstone::GraphicsAPI::Texture *texture = engine.getTextureManager()->getTexture(handle);
		component->cubemap_ = texture;
		Grindstone::GraphicsAPI::SingleTextureBind stb;
		stb.texture = component->cubemap_;
		stb.address = 4;

		Grindstone::GraphicsAPI::TextureBindingCreateInfo ci;
		ci.textures = &stb;
		ci.layout = engine.getSystem<CubemapSystem>()->texture_binding_layout_;
		ci.textureCount = 1;
		component->cubemap_binding_ = engine.getGraphicsWrapper()->createTextureBinding(ci);
	}
}

REFLECT_STRUCT_BEGIN(CubemapComponent, CubemapSystem, COMPONENT_CUBEMAP)
REFLECT_STRUCT_MEMBER_D(capture_method_, "Capture Method", "capturemethod", reflect::Metadata::SaveSetAndView, handleMode)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
