#ifndef _CAMERA_HPP
#define _CAMERA_HPP

#include <glm/glm.hpp>
#include <PostProcess/PostPipeline.hpp>
#include "PostProcess/BasePost.hpp"
#include "Core/GameObject.hpp"

class RenderPath;
class RenderTarget;
class Framebuffer;
class RenderPath;
class Space;

struct RayTraceResults {
	bool hit;
	GameObjectHandle object_handle;
	glm::vec3 position;
};

class Camera {
public:
	Camera(Space *space, bool useFramebuffer = false);
	Camera(Space *space, unsigned int w, unsigned int h, bool useFramebuffer = false);
	void initialize();
	void setViewport(unsigned int w, unsigned int h);
	void setPosition(glm::vec3 pos);
	void setDirections(glm::vec3 fwd, glm::vec3 up);
	void buildProjection();
	void buildView();
	void render();
	void setOrtho(float l, float r, float t, float b);
	void setPerspective();

	void reloadGraphics();
	void destroyGraphics();

	float getFov();
	float getAspectRatio();
	float getNear();
	float getFar();
	const glm::vec3 &getPosition();
	const glm::vec3 &getForward();
	const glm::vec3 &getUp();
	const glm::mat4 &getView();
	const glm::mat4 &getProjection();

	RayTraceResults rayTrace(glm::vec3 pos, glm::vec3 final_pos);
	RayTraceResults rayTraceMousePostion(unsigned int mx, unsigned int my);
	~Camera();

	unsigned int viewport_width_;
	unsigned int viewport_height_;

	bool is_ortho_;
	float ortho_x_;
	float ortho_y_;
	float ortho_width_;
	float ortho_height_;

	float projection_fov_;

	float aspect_ratio_;
	float near_;
	float far_;
	float aperture_size_;
	float shutter_speed_;
	float iso_;
	glm::vec3 position_;
	glm::vec3 forward_;
	glm::vec3 up_;
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
	bool view_dirty_;
	bool projection_dirty_;
	glm::mat4 pv_;
	BasePostProcess *pp_ssao;
};

#endif