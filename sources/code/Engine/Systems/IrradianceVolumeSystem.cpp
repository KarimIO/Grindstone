#include "IrradianceVolumeSystem.hpp"
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

void IrradianceVolumeSystem::prepareCube() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	std::array<glm::vec3, 8> vertices = {
		glm::vec3(-1.0, -1.0, -1.0), glm::vec3( 1.0, -1.0, -1.0),
		glm::vec3( 1.0,  1.0, -1.0), glm::vec3(-1.0,  1.0, -1.0),
		glm::vec3(-1.0, -1.0,  1.0), glm::vec3( 1.0, -1.0,  1.0),
		glm::vec3( 1.0,  1.0,  1.0), glm::vec3(-1.0,  1.0,  1.0)
	};

	std::array<uint32_t, 36> indices = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};

	cube_vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
	});

	Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	Grindstone::GraphicsAPI::VertexBufferCreateInfo cube_vbo_ci;
	cube_vbo_ci.layout = &cube_vertex_layout_;
	cube_vbo_ci.content = vertices.data();
	cube_vbo_ci.count = (uint32_t)vertices.size();
	cube_vbo_ci.size = (uint32_t)(sizeof(glm::vec3) * vertices.size());


	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo cube_vao_ci;
	cube_vbo_ = graphics_wrapper->createVertexBuffer(cube_vbo_ci);
	cube_ibo_ = graphics_wrapper->createIndexBuffer(ibci);
	cube_vao_ci.vertex_buffers = &cube_vbo_;
	cube_vao_ci.vertex_buffer_count = 1;
	cube_vao_ci.index_buffer = cube_ibo_;
	cube_vao_ = graphics_wrapper->createVertexArrayObject(cube_vao_ci);
}

void IrradianceVolumeSystem::prepareUniformBuffer() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "SphericalHarmonicsBuffer";
	ubbci.size = sizeof(SphericalHarmonicsBuffer);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	ubb_ = graphics_wrapper->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(SphericalHarmonicsBuffer);
	ubci.binding = ubb_;
	ub_ = graphics_wrapper->createUniformBuffer(ubci);
}

void IrradianceVolumeSystem::prepareIrradianceShader() {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto settings = engine.getSettings();

	Grindstone::GraphicsAPI::ShaderStageCreateInfo vi;
	Grindstone::GraphicsAPI::ShaderStageCreateInfo fi;
	std::vector<char> vfile;
	std::vector<char> ffile;

	if (settings->graphics_language_ == GraphicsLanguage::OpenGL) {
		vi.fileName = "../assets/shaders/baking/IrradianceVolume_vert.glsl";
		fi.fileName = "../assets/shaders/baking/IrradianceVolume_irr.glsl";
	}
	else if (settings->graphics_language_ == GraphicsLanguage::DirectX) {
		vi.fileName = "../assets/shaders/baking/IrradianceVolume_vert.fxc";
		fi.fileName = "../assets/shaders/baking/IrradianceVolume_irr.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/baking/IrradianceVolume_vert.spv";
		fi.fileName = "../assets/shaders/baking/IrradianceVolume_irr.spv";
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
	irrGPCI.vertex_bindings = &cube_vertex_layout_;
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
IrradianceVolumeComponent::IrradianceVolumeComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_IrradianceVolume, object_handle, id), capture_method_(IrradianceVolumeComponent::CaptureMethod::CAPTURE_BAKE), near_(0.1f), far_(100.0f), resolution_(512), IrradianceVolume_(nullptr), IrradianceVolume_binding_(nullptr), capture_fbo_(nullptr), render_target_(nullptr) {}

IrradianceVolumeSystem::IrradianceVolumeSystem() : System(COMPONENT_IrradianceVolume) {
	GRIND_PROFILE_FUNC();
	auto gw = engine.getGraphicsWrapper();
	auto input = engine.getInputManager();

	cube_binding_ = Grindstone::GraphicsAPI::TextureSubBinding("environmentMap", 4);

	projection_ = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 5.0f);

	loadGraphics();
}

IrradianceVolumeSubSystem::IrradianceVolumeSubSystem(Space *space) : SubSystem(COMPONENT_IrradianceVolume, space) {

	// Prepare Camera
	camera_ = new Camera(space, true);
	camera_->setCustomFinalFramebuffer(engine.getSystem<IrradianceVolumeSystem>()->camera_framebuffer_);
	camera_->projection_fov_ = glm::radians(90.0f);
	camera_->enable_reflections_ = true;
	camera_->enable_auto_exposure_ = false;
	camera_->setViewport(512, 512); // Un-hardcode this
	camera_->initialize();
}

void IrradianceVolumeSubSystem::initialize() {
	for (auto &component : components_) {
	}
}

ComponentHandle IrradianceVolumeSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

void IrradianceVolumeSystem::update() {
	GRIND_PROFILE_FUNC();
	for (auto space : engine.getSpaces()) {
		IrradianceVolumeSubSystem *subsystem = (IrradianceVolumeSubSystem *)space->getSubsystem(system_type_);
		for (auto &component : subsystem->components_) {
		}
	}
}

void IrradianceVolumeSystem::loadGraphics() {
	auto gw = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &cube_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	texture_binding_layout_ = gw->createTextureBindingLayout(tblci);

	prepareCube();
	prepareUniformBuffer();
	prepareIrradianceShader();
	prepareSpecularShader();

	for (auto space : engine.getSpaces()) {
	}
}

void IrradianceVolumeSystem::destroyGraphics() {
	auto gw = engine.getGraphicsWrapper();

	gw->deleteGraphicsPipeline(irradiance_pipeline_);
	gw->deleteRenderTarget(irradiance_image_);
	gw->deleteFramebuffer(irradiance_fbo_);

	gw->deleteVertexArrayObject(cube_vao_);
	gw->deleteVertexBuffer(cube_vbo_);
	gw->deleteIndexBuffer(cube_ibo_);

	gw->deleteUniformBufferBinding(ubb_);
	gw->deleteUniformBuffer(ub_);

}

IrradianceVolumeComponent & IrradianceVolumeSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * IrradianceVolumeSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t IrradianceVolumeSubSystem::getNumComponents() {
	return components_.size();
}

void IrradianceVolumeSubSystem::removeComponent(ComponentHandle handle) {
}

void IrradianceVolumeSystem::loadIrradianceVolumes() {
}

IrradianceVolumeComponent * IrradianceVolumeSubSystem::getClosestIrradianceVolume(glm::vec3 eye) {
	float dist_max = INFINITY;
	IrradianceVolumeComponent *max = nullptr;

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

IrradianceVolumeSubSystem::~IrradianceVolumeSubSystem() {
}

REFLECT_STRUCT_BEGIN(IrradianceVolumeComponent, IrradianceVolumeSystem, COMPONENT_IrradianceVolume)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
