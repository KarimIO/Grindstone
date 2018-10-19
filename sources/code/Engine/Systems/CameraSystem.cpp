#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "Core/Engine.hpp"
#include "CameraSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "glm/gtx/transform.hpp"

CameraComponent::CameraComponent(ComponentHandle id) :
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
	ortho_height_(1.0f) {
	component_type_ = COMPONENT_CAMERA;
	id_ = id;
}

Component * CameraSystem::addComponent() {
	components_.emplace_back(components_.size());
	return &components_.back();
}

void CameraSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

void CameraSystem::update(double dt) {
	const Settings *settings = engine.getSettings();
	double default_aspect = double(settings->resolution_x_) / double(settings->resolution_y_);
	double default_fov = 1.0;

	bool invert_proj = settings->graphics_language_ == GRAPHICS_VULKAN;
	bool scale_proj = settings->graphics_language_ == GRAPHICS_DIRECTX;
	
	for (auto &component : components_) {
		// Get Transform Info
		// TransformSystem &system;
		// TransformComponent &transform = system.get[component.game_object_id_].components_[COMPONENT_TRANSFORM]];
		TransformSystem transform_system;
		ComponentHandle transform_id = 0;

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
		glm::vec3 pos = transform_system.getPosition(transform_id);
		component.view_ = glm::lookAt(
			pos, 
			pos + transform_system.getForward(transform_id),
			transform_system.getUp(transform_id)
		);

		// Culling

		// Render
		engine.getGraphicsPipelineManager()->drawDeferredImmediate();

		// PostProcessing
	}
}

CameraSystem::CameraSystem() {
	this->system_type_ = COMPONENT_CAMERA;
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
