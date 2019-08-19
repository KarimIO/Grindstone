#ifndef _CUBEMAP_SYSTEM_H
#define _CUBEMAP_SYSTEM_H

#include "BaseSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>
#include <vector>
#include "Texture.hpp"

#include "CubeInfo.hpp"

class Camera;
class Framebuffer;
class RenderTarget;
class TextureBindingLayout;
class GraphicsPipeline;
class UniformBuffer;
class VertexArrayObject;
class VertexBufferObject;
class IndexBuffer;

struct CubemapComponent : public Component {
	CubemapComponent(GameObjectHandle object_handle, ComponentHandle id);
	uint32_t resolution_;
	Texture *cubemap_;
	TextureBinding *cubemap_binding_;
	Framebuffer *capture_fbo_;
	RenderTarget *render_target_;
	float near_;
	float far_;
	enum CaptureMethod {
		CAPTURE_REALTIME = 0,
		CAPTURE_BAKE,
		CAPTURE_CUSTOM
	} capture_method_;
	std::string path_;
};

class CubemapSystem : public System {
public:
	CubemapSystem();
	void update(double dt);
};

class CubemapSubSystem : public SubSystem {
	friend CubemapSystem;
public:
	CubemapSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	CubemapComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	CubemapComponent *getClosestCubemap(glm::vec3);

	void bake();

	virtual ~CubemapSubSystem();
private:
	void prepareSphere();
	void prepareUniformBuffer();
	void prepareIrradianceShader();
	void prepareSpecularShader();

	void convoluteIrradiance(CubemapComponent &c);
	void convoluteSpecular(CubemapComponent &c);

	void loadCubemaps();

	std::vector<CubemapComponent> components_;
	TextureSubBinding cube_binding_;
	TextureBindingLayout *texture_binding_layout_;
	Camera *camera_;
	glm::mat4 projection_;

	GraphicsPipeline *irradiance_pipeline_;
	RenderTarget *irradiance_image_;
	Framebuffer *irradiance_fbo_;

	GraphicsPipeline *specular_pipeline_;
	RenderTarget *specular_image_;
	Framebuffer *specular_fbo_;

	VertexArrayObject *sphere_vao_;
	VertexBuffer *sphere_vbo_;
	IndexBuffer *sphere_ibo_;
	VertexBindingDescription sphere_vbd_;
	VertexAttributeDescription sphere_vad_;

	Framebuffer *camera_framebuffer_;
	RenderTarget *final_buffer_;

	unsigned int total_sphere_indices_;

	UniformBufferBinding *ubb_;
	UniformBuffer *ub_;
	struct ConvolutionBufferObject {
		glm::mat4 matrix_;
		float roughness_;
	} ubo_;
};

#endif