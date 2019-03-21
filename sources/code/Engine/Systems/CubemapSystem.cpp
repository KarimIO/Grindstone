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
#include "Systems/CameraSystem.hpp"
#include "Renderpaths/RenderPath.hpp"
#include <thread>

void CubemapSubSystem::bake() {
	TransformSubSystem *transform = (TransformSubSystem *)(space_->getSubsystem(COMPONENT_TRANSFORM));
	CameraSubSystem *cam_sys = (CameraSubSystem *)space_->getSubsystem(COMPONENT_CAMERA);
	auto cam = cam_sys->getComponent(0);

	unsigned char **data = new unsigned char*[6];

	for (auto &component : components_) {
		glm::mat4 proj = glm::perspective(1.5708f, 1.0f, component.near_, component.far_);
		glm::mat4 view;

		GameObjectHandle game_object_id = component.game_object_handle_;
		GameObject &obj = space_->getObject(game_object_id);
		ComponentHandle transform_id = obj.getComponentHandle(COMPONENT_TRANSFORM);

		for (uint8_t i = 0; i < 6; ++i) {
			data[i] = new unsigned char[component.resolution_ * component.resolution_ * 4];
			glm::vec3 pos = transform->getPosition(transform_id);
			view = glm::lookAt(pos, transform->getPosition(transform_id) + gCubeDirections[i].Target, gCubeDirections[i].Up);
			// cam->getComponent(camera_id).

			glm::mat4 pv = proj * view;

			auto ubo = engine.getUniformBuffer();
			ubo->Bind();
			ubo->UpdateUniformBuffer(&pv);

			// Culling
			//engine.ubo2->Bind();


			Engine::DefferedUBO deferred_ubo;
			deferred_ubo.invProj = glm::inverse(proj);
			deferred_ubo.view = glm::inverse(view);
			deferred_ubo.eyePos.x = pos.x;
			deferred_ubo.eyePos.y = pos.y;
			deferred_ubo.eyePos.z = pos.z;
			deferred_ubo.resolution.x = deferred_ubo.resolution.y = component.resolution_;
			engine.deff_ubo_handler_->UpdateUniformBuffer(&deferred_ubo);

			engine.getGraphicsWrapper()->setViewport(0, 0, component.resolution_, component.resolution_);

			Framebuffer *gbuffer = nullptr; // component.capture_fbo_;
			// FIX THIS: ((CameraSystem *) engine.getSystem(COMPONENT_CAMERA))->render_path_->render(gbuffer, space_);

			// PostProcessing
			//engine.getGraphicsWrapper()->BindVertexArrayObject(engine.getPlaneVAO());
			//component.post_pipeline_.Process();

			engine.getGraphicsWrapper()->SwapBuffer();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			//gbuffer->BindRead();
			//component.render_target_->RenderScreen(0, component.resolution_, component.resolution_, data[i]);
		}
		
		/*std::string path = std::string("../assets/cubemaps/") + component.path_;
		engine.getTextureManager()->writeCubemap(path, data, component.resolution_);
		std::cout << "Outputting " << path << "\n";*/
	}
}

CubemapComponent::CubemapComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_CUBEMAP, object_handle, id), capture_method_(CubemapComponent::CaptureMethod::CAPTURE_BAKE), near_(0.1f), far_(100.0f), resolution_(128) {}

void CubemapComponent::bake() {
}

CubemapSystem::CubemapSystem() : System(COMPONENT_CUBEMAP) {
	auto input = engine.getInputManager();
	input->AddControl("q", "CaptureCubemaps", NULL, 1);
	input->BindAction("CaptureCubemaps", NULL, this, &CubemapSystem::bake, KEY_RELEASED);
}

void CubemapSystem::bake(double t) {
	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			CubemapSubSystem *subsystem = (CubemapSubSystem *)space->getSubsystem(system_type_);
			subsystem->bake();
		}
	}
}

CubemapSubSystem::CubemapSubSystem(Space *space) : SubSystem(COMPONENT_CUBEMAP, space) {
	cube_binding_ = TextureSubBinding("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 4;
	tblci.bindings = &cube_binding_;
	tblci.bindingCount = (uint32_t)1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	texture_binding_layout_ = engine.getGraphicsWrapper()->CreateTextureBindingLayout(tblci);
}

ComponentHandle CubemapSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

ComponentHandle CubemapSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	if (params.HasMember("type")) {
		std::string type = params["type"].GetString();

		if (type == "baked") {
			component.capture_method_ = CubemapComponent::CaptureMethod::CAPTURE_BAKE;

			auto objname = space_->getObject(object_handle).getName();
			component.path_ = objname + ".png";

			// Load File
			TextureHandler handle = engine.getTextureManager()->loadCubemap(std::string("../assets/cubemaps/") + component.path_);
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
				LOG_WARN("No path given.");
			}
		}
		else {
			LOG_WARN("Invalid type.");
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

	return component_handle;
}


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

CubemapComponent & CubemapSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t CubemapSubSystem::getNumComponents() {
	return components_.size();
}

void CubemapSubSystem::removeComponent(ComponentHandle handle) {
}

void CubemapSubSystem::loadCubemaps() {
}

CubemapComponent * CubemapSubSystem::getClosestCubemap(glm::vec3 eye) {
	return nullptr;

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
