#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "Core/Engine.hpp"
#include "CameraSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "glm/gtx/transform.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include "GraphicsWrapper.hpp"
#include "./Renderpaths/RenderPathDeferred.hpp"

CameraComponent::CameraComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_CAMERA, object_handle, handle),
	aperture_size_(16),
	shutter_speed_(1.0f / 200.0f),
	iso_(200.0f),
	near_(0.1f),
	far_(100.0f),
	projection_fov_(1.0f),
	viewport_x_(0.0f),
	viewport_y_(0.0f),
	viewport_width_(1.0f),
	viewport_height_(1.0f),
	is_ortho(false),
	ortho_x_(0.0f),
	ortho_y_(0.0f),
	ortho_width_(1.0f),
	ortho_height_(1.0f) {}

CameraSubSystem::CameraSubSystem() : SubSystem(COMPONENT_CAMERA) {
}

ComponentHandle CameraSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	//auto component = components_.back();

	return component_handle;
}

CameraComponent & CameraSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

void CameraSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

CameraSubSystem::~CameraSubSystem() {}

void CameraSystem::update(double dt) {
	const Settings *settings = engine.getSettings();
	double default_aspect = double(settings->resolution_x_) / double(settings->resolution_y_);
	double default_fov = 1.0;

	bool invert_proj = settings->graphics_language_ == GRAPHICS_VULKAN;
	bool scale_proj = settings->graphics_language_ == GRAPHICS_DIRECTX;

	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			CameraSubSystem *subsystem = (CameraSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				GameObjectHandle game_object_id = component.game_object_handle_;

				// Get Transform Info
				ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
				TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

				// Calculate Projection
				if (component.is_ortho) {
					// Perspective
					component.projection_ = glm::ortho(component.ortho_x_, component.ortho_y_, component.ortho_width_, component.ortho_height_);
				}
				else {
					// Orthographic
					double fov = default_fov * component.projection_fov_;
					double aspect = default_aspect * (component.viewport_width_ / component.viewport_height_);
					component.projection_ = glm::perspective(fov, aspect, component.near_, component.far_);
				}

				if (invert_proj)
					component.projection_[1][1] *= -1;

				if (scale_proj) {
					const glm::mat4 scale = glm::mat4(1.0f, 0, 0, 0,
						0, 1.0f, 0, 0,
						0, 0, 0.5f, 0,
						0, 0, 0.25f, 1.0f);

					component.projection_ = scale * component.projection_;
				}

				// CalculateView
				glm::vec3 pos = transform->getPosition(transform_id);
				component.view_ = glm::lookAt(
					pos,
					pos + transform->getForward(transform_id),
					transform->getUp(transform_id)
				);

				glm::mat4 pv = component.projection_ * component.view_;

				ubo_->Bind();
				ubo_->UpdateUniformBuffer(&pv);

				// Culling
				//engine.ubo2->Bind();

				Framebuffer *gbuffer = nullptr; // engine.getDefaultFramebuffer()
				render_path_->render(gbuffer);

				// PostProcessing
				engine.getGraphicsWrapper()->SwapBuffer();
			}
		}
	}
}

CameraSystem::CameraSystem() : System(COMPONENT_CAMERA) {
	render_path_ = new RenderPathDeferred();

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = 128;
	ubci.binding = engine.getUniformBufferBinding();
	ubo_ = engine.getGraphicsWrapper()->CreateUniformBuffer(ubci);
	/*RenderTargetContainer *rt_hdr = &engine.rt_hdr_;

	if (engine.settings.use_ssao) {
		BasePostProcess *pp_ssao = new PostProcessSSAO(&engine.rt_gbuffer_);
		post_pipeline_.AddPostProcess(pp_ssao); 
	}

	if (engine.settings.enableReflections) {
		BasePostProcess *pp_ibl = new PostProcessIBL(&engine.rt_gbuffer_, rt_hdr);
		post_pipeline_.AddPostProcess(pp_ibl);
	}

	PostProcessAutoExposure *pp_auto = new PostProcessAutoExposure(rt_hdr, nullptr);
	post_pipeline_.AddPostProcess(pp_auto);

	PostProcessTonemap *pp_tonemap = new PostProcessTonemap(rt_hdr, nullptr, pp_auto);
	post_pipeline_.AddPostProcess(pp_tonemap);*/
}
