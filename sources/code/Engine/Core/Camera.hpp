#ifndef _CAMERA_HPP
#define _CAMERA_HPP

#include <glm/glm.hpp>
#include <PostProcess/PostPipeline.hpp>
#include "PostProcess/BasePost.hpp"

class RenderPath;
class RenderTarget;
class Framebuffer;
class RenderPath;
class Space;

class Camera {
public:
	Camera(Space *space, bool useFramebuffer = false);
	Camera(Space *space, unsigned int w, unsigned int h, bool useFramebuffer = false);
	void initialize();
	void setViewport(unsigned int w, unsigned int h);
	void render(glm::vec3 &pos, glm::mat4 &view);
	void render(glm::vec3 &pos, glm::mat4 &view, int face);
	void setOrtho(double l, double r, double t, double b);
	void setPerspective();
	~Camera();

	unsigned int viewport_width_;
	unsigned int viewport_height_;

	bool is_ortho;
	double ortho_x_;
	double ortho_y_;
	double ortho_width_;
	double ortho_height_;

	double projection_fov_;

	double near_;
	double far_;
	double aperture_size_;
	double shutter_speed_;
	double iso_;
	glm::mat4 projection_;
	glm::mat4 view_;

	PostPipeline post_pipeline_;

	RenderTargetContainer rt_hdr_;
	RenderTarget *hdr_buffer_;
	DepthTarget *depth_target_;
	Framebuffer *hdr_framebuffer_;
	RenderPath *render_path_;

	RenderTarget *final_buffer_;
	Framebuffer *final_framebuffer_;

	bool custom_final_framebuffer_;
	void setCustomFinalFramebuffer(Framebuffer *framebuffer);

	bool enable_reflections_;
	bool enable_auto_exposure_;
	
	bool use_framebuffer_;

	Space *space_;

	void setEnabled(bool);
private:
	void generateFramebuffers();

	bool enabled_;
	BasePostProcess *pp_ssao;
};

#endif