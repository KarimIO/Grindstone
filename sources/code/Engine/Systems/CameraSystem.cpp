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

#include "PostProcess/PostProcessTonemap.hpp"
#include "PostProcess/PostProcessAutoExposure.hpp"
#include "PostProcess/PostProcessSSAO.hpp"
#include "PostProcess/PostProcessIBL.hpp"

CameraComponent::CameraComponent(Space *space, GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_CAMERA, object_handle, handle),
	post_pipeline_(space),
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

CameraSubSystem::CameraSubSystem(Space *space) : SubSystem(COMPONENT_CAMERA, space) {
}

ComponentHandle CameraSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(space_, object_handle, component_handle);
	auto &component = components_.back();
	auto settings = engine.getSettings();
	auto graphics_wrapper = engine.getGraphicsWrapper();

	std::vector<RenderTargetCreateInfo> hdr_buffer_ci;
	hdr_buffer_ci.reserve(1);
	hdr_buffer_ci.emplace_back(FORMAT_COLOR_R16G16B16, settings->resolution_x_, settings->resolution_y_);
	component.hdr_buffer_ = graphics_wrapper->CreateRenderTarget(hdr_buffer_ci.data(), hdr_buffer_ci.size());

	FramebufferCreateInfo hdr_framebuffer_ci;
	hdr_framebuffer_ci.render_target_lists = &component.hdr_buffer_;
	hdr_framebuffer_ci.num_render_target_lists = 1;
	hdr_framebuffer_ci.depth_target = nullptr; // depth_image_;
	hdr_framebuffer_ci.render_pass = nullptr;
	component.hdr_framebuffer_ = graphics_wrapper->CreateFramebuffer(hdr_framebuffer_ci);

	component.rt_hdr_.framebuffer = component.hdr_framebuffer_;
	component.rt_hdr_.render_targets = &component.hdr_buffer_;
	component.rt_hdr_.num_render_targets = 1;
	component.rt_hdr_.depth_target = nullptr;

	//RenderTargetContainer *rt_hdr = &engine.rt_hdr_;

	if (engine.getSettings()->enable_ssao_) {
		BasePostProcess *pp_ssao = new PostProcessSSAO(&component.post_pipeline_, &component.rt_hdr_);
		component.post_pipeline_.AddPostProcess(pp_ssao);
	}

	if (settings->enable_reflections_) {
		PostProcessIBL *pp_ibl = new PostProcessIBL(&component.post_pipeline_, &component.rt_hdr_);
		component.post_pipeline_.AddPostProcess(pp_ibl);
	}

	PostProcessAutoExposure *pp_auto = new PostProcessAutoExposure(&component.post_pipeline_, &component.rt_hdr_, nullptr);
	component.post_pipeline_.AddPostProcess(pp_auto);

	PostProcessTonemap *pp_tonemap = new PostProcessTonemap(&component.post_pipeline_, &component.rt_hdr_, nullptr, pp_auto);
	component.post_pipeline_.AddPostProcess(pp_tonemap);

	return component_handle;
}

CameraComponent & CameraSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t CameraSubSystem::getNumComponents() {
	return components_.size();
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

				auto ubo = engine.getUniformBuffer();
				ubo->Bind();
				ubo->UpdateUniformBuffer(&pv);

				// Culling
				//engine.ubo2->Bind();


				Engine::DefferedUBO deferred_ubo;
				deferred_ubo.invProj = glm::inverse(component.projection_);
				deferred_ubo.view = glm::inverse(component.view_);
				deferred_ubo.eyePos.x = pos.x;
				deferred_ubo.eyePos.y = pos.y;
				deferred_ubo.eyePos.z = pos.z;
				deferred_ubo.resolution.x = engine.getSettings()->resolution_x_;
				deferred_ubo.resolution.y = engine.getSettings()->resolution_y_;
				engine.deff_ubo_handler_->UpdateUniformBuffer(&deferred_ubo);

				Framebuffer *gbuffer = component.hdr_framebuffer_;
				render_path_->render(gbuffer, space);

				// PostProcessing
				engine.getGraphicsWrapper()->BindVertexArrayObject(engine.getPlaneVAO());
				component.post_pipeline_.Process();

				engine.getGraphicsWrapper()->SwapBuffer();
			}
		}
	}
}

CameraSystem::CameraSystem() : System(COMPONENT_CAMERA) {
	render_path_ = new RenderPathDeferred();
}
