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
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "./Renderpaths/RenderPathDeferred.hpp"

#include "PostProcess/PostProcessTonemap.hpp"
#include "PostProcess/PostProcessAutoExposure.hpp"
#include "PostProcess/PostProcessIBL.hpp"
#include "PostProcess/PostProcessColorGrading.hpp"
#include "PostProcess/PostProcessBloom.hpp"
#include "PostProcess/PostProcessSSR.hpp"
#include "../Systems/RenderSpriteSystem.hpp"

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
	is_ortho_(false),
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
	aspect_ratio_ = float(w) / float(h);
}

void Camera::initialize() {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	render_path_ = new RenderPathDeferred(viewport_width_, viewport_height_);
	post_pipeline_.setSpace(space_);

	if (false) {
		PostProcessSSR *pp_ssr = new PostProcessSSR(viewport_width_, viewport_height_, &post_pipeline_, &rt_hdr_, &rt_hdr_);
		post_pipeline_.AddPostProcess(pp_ssr);
	}

	if (false && enable_reflections_) {
		PostProcessIBL *pp_ibl = new PostProcessIBL(viewport_width_, viewport_height_, &post_pipeline_, &rt_hdr_);
		post_pipeline_.AddPostProcess(pp_ibl);
	}
	
	PostProcessAutoExposure *pp_auto = nullptr;
	if (false && enable_auto_exposure_) {
		pp_auto = new PostProcessAutoExposure(viewport_width_, viewport_height_, &post_pipeline_, &rt_hdr_, nullptr);
		post_pipeline_.AddPostProcess(pp_auto);
	}

	//PostProcessBloom *pp_bloom = new PostProcessBloom(viewport_width_, viewport_height_, &post_pipeline_, &rt_hdr_, &rt_hdr_, pp_auto);
	//post_pipeline_.AddPostProcess(pp_bloom);

	PostProcessTonemap *pp_tonemap = new PostProcessTonemap(viewport_width_, viewport_height_, &post_pipeline_, &rt_hdr_, &rt_hdr_, pp_auto);
	post_pipeline_.AddPostProcess(pp_tonemap);

	PostProcessColorGrading *pp_grading = new PostProcessColorGrading(viewport_width_, viewport_height_, &post_pipeline_, &rt_hdr_, &final_framebuffer_);
	post_pipeline_.AddPostProcess(pp_grading);
}

void Camera::setEnabled(bool status) {
	enabled_ = status;
}

void Camera::setCustomFinalFramebuffer(Grindstone::GraphicsAPI::Framebuffer *framebuffer) {
	final_framebuffer_ = framebuffer;
	custom_final_framebuffer_ = true;
}

void Camera::generateFramebuffers() {
	if (hdr_buffer_) {
		engine.getGraphicsWrapper()->deleteRenderTarget(hdr_buffer_);
	}

	if (hdr_framebuffer_) {
		engine.getGraphicsWrapper()->deleteFramebuffer(hdr_framebuffer_);
	}

	Grindstone::GraphicsAPI::RenderTargetCreateInfo hdr_buffer_ci(Grindstone::GraphicsAPI::ColorFormat::R16G16B16, viewport_width_, viewport_height_);
	hdr_buffer_ = engine.getGraphicsWrapper()->createRenderTarget(&hdr_buffer_ci, 1);

	Grindstone::GraphicsAPI::DepthTargetCreateInfo depth_image_ci(Grindstone::GraphicsAPI::DepthFormat::D24_STENCIL_8, viewport_width_, viewport_height_, false, false);
	depth_target_ = engine.getGraphicsWrapper()->createDepthTarget(depth_image_ci);

	Grindstone::GraphicsAPI::FramebufferCreateInfo hdr_framebuffer_ci;
	hdr_framebuffer_ci.render_target_lists = &hdr_buffer_;
	hdr_framebuffer_ci.num_render_target_lists = 1;
	hdr_framebuffer_ci.depth_target = depth_target_;
	hdr_framebuffer_ci.render_pass = nullptr;
	hdr_framebuffer_ = engine.getGraphicsWrapper()->createFramebuffer(hdr_framebuffer_ci);

	if (use_framebuffer_ && !custom_final_framebuffer_) {
		/*if (final_buffer_) {
			engine.getGraphicsWrapper()->deleteRenderTarget(final_buffer_);
		}*/

		if (final_framebuffer_) {
			engine.getGraphicsWrapper()->deleteFramebuffer(final_framebuffer_);
		}

		Grindstone::GraphicsAPI::RenderTargetCreateInfo fbo_buffer_ci(Grindstone::GraphicsAPI::ColorFormat::R8G8B8, viewport_width_, viewport_height_);
		final_buffer_ = engine.getGraphicsWrapper()->createRenderTarget(&fbo_buffer_ci, 1);

		//DepthTargetCreateInfo depth_image_ci2(Grindstone::GraphicsAPI::DepthFormat::D24_STENCIL_8, viewport_width_, viewport_height_, false, false);
		//DepthTarget *depth_target2_ = engine.getGraphicsWrapper()->createDepthTarget(depth_image_ci);

		Grindstone::GraphicsAPI::FramebufferCreateInfo final_framebuffer_ci;
		final_framebuffer_ci.render_target_lists = &final_buffer_;
		final_framebuffer_ci.num_render_target_lists = 1;
		final_framebuffer_ci.depth_target = nullptr; // depth_image_;
		final_framebuffer_ci.render_pass = nullptr;
		final_framebuffer_ = engine.getGraphicsWrapper()->createFramebuffer(final_framebuffer_ci);
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

		aspect_ratio_ = float(w) / float(h);
		projection_dirty_ = true;

		generateFramebuffers();
		post_pipeline_.resizeBuffers(viewport_width_, viewport_height_);
	}
}

void Camera::setPosition(glm::vec3 pos) {
	position_ = pos;

	view_dirty_ = true;
}

void Camera::setDirections(glm::vec3 fwd, glm::vec3 up) {
	forward_ = fwd;
	up_ = up;

	view_dirty_ = true;
}

void Camera::buildProjection() {
	const Settings *settings = engine.getSettings();

	bool invert_proj = settings->graphics_language_ == GraphicsLanguage::Vulkan;
	bool scale_proj = settings->graphics_language_ == GraphicsLanguage::DirectX;

	// Calculate Projection
	if (is_ortho_) {
		// Perspective
		;

		float z = 3.0f;
		float x = z * aspect_ratio_;
		float y = z;
		projection_ = glm::ortho(-x, x, -y, y, 0.5f, 50.0f);
	}
	else {
		// Orthographic
		projection_ = glm::perspective(projection_fov_, aspect_ratio_, near_, far_);
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

	projection_dirty_ = false;
}

void Camera::buildView() {
	view_ = glm::lookAt(position_, position_ + forward_, up_);

	view_dirty_ = false;
}

struct UBOProjView {
	glm::mat4 proj_view;
	glm::vec3 eyepos;
	float time;
	glm::mat4 invproj;
	glm::mat4 invview;
	glm::vec4 resolution;
};

void Camera::render() {
	GRIND_PROFILE_FUNC();
	if (space_ == nullptr || !enabled_) return;

	bool dirty = projection_dirty_ || view_dirty_;

	if (projection_dirty_) {
		buildProjection();
	}

	if (view_dirty_) {
		buildView();
	}

	if (dirty) {
		pv_ = projection_ * view_;
	}

	auto graphics_wrapper = engine.getGraphicsWrapper();
	
	// Get Transform Info
	//ComponentHandle transform_id = space->getObject(game_object_id).getComponentHandle(COMPONENT_TRANSFORM);
	//TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

	auto ubo = engine.getUniformBuffer();
	ubo->Bind();
	UBOProjView ubodata;
	ubodata.proj_view = pv_;
	ubodata.time = (float)engine.getTimeCurrent();
	ubodata.eyepos = position_;
	ubodata.invproj = glm::inverse(projection_);
	ubodata.invview = glm::inverse(view_);
	ubodata.resolution.x = (float)viewport_width_;
	ubodata.resolution.y = (float)viewport_height_;
	ubo->updateBuffer(&ubodata);

	// Culling
	//engine.ubo2->Bind();
	 

	Engine::DefferedUBO deferred_ubo;
	deferred_ubo.invProj = ubodata.invproj;
	deferred_ubo.view = ubodata.invview;
	deferred_ubo.eyePos.x = position_.x;
	deferred_ubo.eyePos.y = position_.y;
	deferred_ubo.eyePos.z = position_.z;
	deferred_ubo.resolution.x = (float)viewport_width_;
	deferred_ubo.resolution.y = (float)viewport_height_;
	engine.deff_ubo_handler_->updateBuffer(&deferred_ubo);

	graphics_wrapper->setViewport(0, 0, viewport_width_, viewport_height_);

	bool in_debug = (render_path_->getDebugMode() > 0);
	Grindstone::GraphicsAPI::Framebuffer *f = in_debug ? final_framebuffer_ : hdr_framebuffer_;
	render_path_->render(f, depth_target_, space_);
	
	// final_framebuffer_->

	// PostProcessing
	if (!in_debug) {
		graphics_wrapper->bindVertexArrayObject(engine.getPlaneVAO());
		post_pipeline_.Process();
	}

	//auto sprite_sys = ((RenderSpriteSubSystem *)space_->getSubsystem(COMPONENT_RENDER_SPRITE));
	//sprite_sys->renderSprites(is_ortho_, position_, depth_target_);
}

void Camera::setOrtho(float l, float r, float t, float b) {
	is_ortho_ = true;
	ortho_x_ = l;
	ortho_y_ = r;
	ortho_width_ = b;
	ortho_height_ = t;
	projection_dirty_ = true;
}

void Camera::setPerspective() {
	is_ortho_ = false;
	projection_dirty_ = true;
}

void Camera::reloadGraphics() {
	render_path_->reloadGraphics();
	generateFramebuffers();

	post_pipeline_.reloadGraphics(viewport_width_, viewport_height_);
}

void Camera::destroyGraphics() {
	if (use_framebuffer_ && !custom_final_framebuffer_) {
		if (final_framebuffer_) {
			engine.getGraphicsWrapper()->deleteFramebuffer(final_framebuffer_);
			final_framebuffer_ = nullptr;
		}

		if (final_buffer_) {
			engine.getGraphicsWrapper()->deleteRenderTarget(final_buffer_);
			final_buffer_ = nullptr;
		}
	}

	if (hdr_framebuffer_) {
		engine.getGraphicsWrapper()->deleteFramebuffer(hdr_framebuffer_);
		hdr_framebuffer_ = nullptr;
	}

	if (hdr_buffer_) {
		engine.getGraphicsWrapper()->deleteRenderTarget(hdr_buffer_);
		hdr_buffer_ = nullptr;
	}

	if (depth_target_) {
		engine.getGraphicsWrapper()->deleteDepthTarget(depth_target_);
		depth_target_ = nullptr;
	}

	post_pipeline_.destroyGraphics();
	RenderPathDeferred *drp = (RenderPathDeferred *)render_path_;
	drp->destroyGraphics();
}

float Camera::getFov() {
	return projection_fov_;
}

float Camera::getAspectRatio() {
	return aspect_ratio_;
}

float Camera::getNear() {
	return near_;
}

float Camera::getFar() {
	return far_;
}

const glm::vec3 & Camera::getPosition() {
	return position_;
}

const glm::vec3 & Camera::getForward() {
	return forward_;
}

const glm::vec3 & Camera::getUp() {
	return up_;
}

const glm::mat4 & Camera::getView() {
	return view_;
}

const glm::mat4 & Camera::getProjection() {
	return projection_;
}

RayTraceResults Camera::rayTrace(glm::vec3 pos, glm::vec3 final_pos)
{
	return RayTraceResults();
}

RayTraceResults Camera::rayTraceMousePostion(unsigned int mx, unsigned int my)
{
	return RayTraceResults();
}

Camera::~Camera() {
	destroyGraphics();
	delete render_path_;
}
