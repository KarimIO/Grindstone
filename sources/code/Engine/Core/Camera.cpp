#include "Camera.hpp"

#include "AssetManagers/GraphicsPipelineManager.hpp"
#include "Core/Engine.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/TransformSystem.hpp"
#include "Core/Engine.hpp"
#include "Utilities/SettingsFile.hpp"
#include "glm/gtx/transform.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include "GraphicsWrapper.hpp"
#include "./Renderpaths/RenderPathDeferred.hpp"

#include "PostProcess/PostProcessTonemap.hpp"
#include "PostProcess/PostProcessAutoExposure.hpp"
#include "PostProcess/PostProcessIBL.hpp"
#include "PostProcess/PostProcessColorGrading.hpp"
#include "PostProcess/PostProcessBloom.hpp"
#include "PostProcess/PostProcessSSR.hpp"

#include <GL/gl3w.h>

Camera::Camera(Space *space, bool useFramebuffer) :
	space_(space),
	post_pipeline_(nullptr),
	aperture_size_(16),
	shutter_speed_(1.0f / 200.0f),
	iso_(200.0f),
	near_(0.1f),
	far_(100.0f),
	projection_fov_(1.0f),
	is_ortho(false),
	ortho_x_(0.0f),
	ortho_y_(0.0f),
	ortho_width_(1.0f),
	ortho_height_(1.0f),
	use_framebuffer_(useFramebuffer),
	hdr_buffer_(nullptr),
	hdr_framebuffer_(nullptr),
	final_framebuffer_(nullptr),
	final_buffer_(nullptr),
	render_path_(nullptr),
	pp_ssao(nullptr),
	custom_final_framebuffer_(false),
	enabled_(true) {
}

Camera::Camera(Space *space, unsigned int w, unsigned int h, bool useFramebuffer) : Camera(space, useFramebuffer) {
	viewport_width_	= w;
	viewport_height_ = h;
}

void Camera::initialize() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	render_path_ = new RenderPathDeferred(viewport_width_, viewport_height_);
	post_pipeline_.setSpace(space_);

	if (true) {
		PostProcessSSR *pp_ssr = new PostProcessSSR(&post_pipeline_, &rt_hdr_, &rt_hdr_);
		post_pipeline_.AddPostProcess(pp_ssr);
	}

	if (false && enable_reflections_) {
		PostProcessIBL *pp_ibl = new PostProcessIBL(&post_pipeline_, &rt_hdr_, viewport_width_, viewport_height_);
		post_pipeline_.AddPostProcess(pp_ibl);
	}
	
	PostProcessAutoExposure *pp_auto = nullptr;
	if (enable_auto_exposure_) {
		pp_auto = new PostProcessAutoExposure(&post_pipeline_, &rt_hdr_, nullptr);
		post_pipeline_.AddPostProcess(pp_auto);
	}

	/*PostProcessBloom *pp_bloom = new PostProcessBloom(&post_pipeline_, &rt_hdr_, &rt_hdr_, pp_auto);
	post_pipeline_.AddPostProcess(pp_bloom);*/

	PostProcessTonemap *pp_tonemap = new PostProcessTonemap(&post_pipeline_, &rt_hdr_, &rt_hdr_, pp_auto);
	post_pipeline_.AddPostProcess(pp_tonemap);

	PostProcessColorGrading *pp_grading = new PostProcessColorGrading(&post_pipeline_, &rt_hdr_, &final_framebuffer_);
	post_pipeline_.AddPostProcess(pp_grading);
}

void Camera::setEnabled(bool status) {
	enabled_ = status;
}

void Camera::setCustomFinalFramebuffer(Framebuffer *framebuffer) {
	final_framebuffer_ = framebuffer;
	custom_final_framebuffer_ = true;
}

void Camera::generateFramebuffers() {
	/*if (hdr_buffer_) {
		engine.getGraphicsWrapper()->DeleteRenderTarget(hdr_buffer_);
	}*/

	if (hdr_framebuffer_) {
		engine.getGraphicsWrapper()->DeleteFramebuffer(hdr_framebuffer_);
	}

	RenderTargetCreateInfo hdr_buffer_ci(FORMAT_COLOR_R16G16B16, viewport_width_, viewport_height_);
	hdr_buffer_ = engine.getGraphicsWrapper()->CreateRenderTarget(&hdr_buffer_ci, 1);

	DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24_STENCIL_8, viewport_width_, viewport_height_, false, false);
	depth_target_ = engine.getGraphicsWrapper()->CreateDepthTarget(depth_image_ci);

	FramebufferCreateInfo hdr_framebuffer_ci;
	hdr_framebuffer_ci.render_target_lists = &hdr_buffer_;
	hdr_framebuffer_ci.num_render_target_lists = 1;
	hdr_framebuffer_ci.depth_target = depth_target_;
	hdr_framebuffer_ci.render_pass = nullptr;
	hdr_framebuffer_ = engine.getGraphicsWrapper()->CreateFramebuffer(hdr_framebuffer_ci);

	if (use_framebuffer_ && !custom_final_framebuffer_) {
		/*if (final_buffer_) {
			engine.getGraphicsWrapper()->DeleteRenderTarget(final_buffer_);
		}*/

		if (final_framebuffer_) {
			engine.getGraphicsWrapper()->DeleteFramebuffer(final_framebuffer_);
		}

		RenderTargetCreateInfo fbo_buffer_ci(FORMAT_COLOR_R8G8B8, viewport_width_, viewport_height_);
		final_buffer_ = engine.getGraphicsWrapper()->CreateRenderTarget(&fbo_buffer_ci, 1);

		//DepthTargetCreateInfo depth_image_ci2(FORMAT_DEPTH_24_STENCIL_8, viewport_width_, viewport_height_, false, false);
		//DepthTarget *depth_target2_ = engine.getGraphicsWrapper()->CreateDepthTarget(depth_image_ci);

		FramebufferCreateInfo final_framebuffer_ci;
		final_framebuffer_ci.render_target_lists = &final_buffer_;
		final_framebuffer_ci.num_render_target_lists = 1;
		final_framebuffer_ci.depth_target = nullptr; // depth_image_;
		final_framebuffer_ci.render_pass = nullptr;
		final_framebuffer_ = engine.getGraphicsWrapper()->CreateFramebuffer(final_framebuffer_ci);
	}

	rt_hdr_.framebuffer = hdr_framebuffer_;
	rt_hdr_.render_targets = &hdr_buffer_;
	rt_hdr_.num_render_targets = 1;
	rt_hdr_.depth_target = nullptr;

	// TODO: Re-Add this
	// if (pp_ssao)
	// 	((PostProcessSSAO *)pp_ibl)->recreateFramebuffer(viewport_width_, viewport_height_);

	if (render_path_)
		render_path_->recreateFramebuffer(viewport_width_, viewport_height_);
}

void Camera::setViewport(unsigned int w, unsigned int h) {
	if (viewport_width_ != w || viewport_height_ != h) {
		viewport_width_ = w;
		viewport_height_ = h;

		generateFramebuffers();
	}
}

void Camera::render(glm::vec3 & pos, glm::mat4 & view) {
	render(pos, view, -1);
}

struct UBOProjView {
	glm::mat4 proj_view;
	glm::vec3 eyepos;
	float time;
	glm::mat4 invproj;
	glm::mat4 invview;
	glm::vec4 resolution;
};

void Camera::render(glm::vec3 &pos, glm::mat4 &view, int face) {
	if (space_ == nullptr || !enabled_) return;

	auto graphics_wrapper = engine.getGraphicsWrapper();

	const Settings *settings = engine.getSettings();
	double default_aspect = double(viewport_width_) / double(viewport_height_);
	double default_fov = 1.0;

	bool invert_proj = settings->graphics_language_ == GRAPHICS_VULKAN;
	bool scale_proj = settings->graphics_language_ == GRAPHICS_DIRECTX;

	// Get Transform Info
	//ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
	//TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

	// Calculate Projection
	if (is_ortho) {
		// Perspective
		projection_ = glm::ortho(ortho_x_, ortho_y_, ortho_width_, ortho_height_, 0.5, 50.0);
	}
	else {
		// Orthographic
		double fov = default_fov * projection_fov_;
		double aspect = default_aspect * (viewport_width_ / viewport_height_);
		projection_ = glm::perspective(fov, aspect, near_, far_);
	}

	if (invert_proj)
		projection_[1][1] *= -1;

	if (scale_proj) {
		const glm::mat4 scale = glm::mat4(1.0f, 0, 0, 0,
			0, 1.0f, 0, 0,
			0, 0, 0.5f, 0,
			0, 0, 0.25f, 1.0f);

		projection_ = scale * projection_;
	}

	glm::mat4 pv = projection_ * view;

	auto ubo = engine.getUniformBuffer();
	ubo->Bind();
	UBOProjView ubodata;
	ubodata.proj_view = pv;
	ubodata.time = engine.getTimeCurrent();
	ubodata.eyepos = pos;
	ubodata.invproj = glm::inverse(projection_);
	ubodata.invview = glm::inverse(view);
	ubodata.resolution.x = viewport_width_;
	ubodata.resolution.y = viewport_height_;
	ubo->UpdateUniformBuffer(&ubodata);

	// Culling
	//engine.ubo2->Bind();
	 

	Engine::DefferedUBO deferred_ubo;
	deferred_ubo.invProj = ubodata.invproj;
	deferred_ubo.view = ubodata.invview;
	deferred_ubo.eyePos.x = pos.x;
	deferred_ubo.eyePos.y = pos.y;
	deferred_ubo.eyePos.z = pos.z;
	deferred_ubo.resolution.x = viewport_width_;
	deferred_ubo.resolution.y = viewport_height_;
	engine.deff_ubo_handler_->UpdateUniformBuffer(&deferred_ubo);

	graphics_wrapper->setViewport(0, 0, viewport_width_, viewport_height_);

	bool in_debug = (render_path_->getDebugMode() > 0);
	Framebuffer *f = in_debug ? final_framebuffer_ : hdr_framebuffer_;
	render_path_->render(f, depth_target_, space_);
	
	// final_framebuffer_->

	// PostProcessing
	if (!in_debug) {
		graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());
		post_pipeline_.Process();
	}
}

void Camera::setOrtho(double l, double r, double t, double b) {
	is_ortho = true;
	ortho_x_ = l;
	ortho_y_ = r;
	ortho_width_ = b;
	ortho_height_ = t;
	//projection_ = glm::ortho(l, r, b, t);
}

void Camera::setPerspective() {
	is_ortho = false;
}

Camera::~Camera() {
}
