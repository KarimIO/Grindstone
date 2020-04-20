#ifndef _CUBEMAP_SYSTEM_H
#define _CUBEMAP_SYSTEM_H

#include "BaseSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>
#include <vector>
#include <GraphicsCommon/Texture.hpp>

#include "CubeInfo.hpp"

class Camera;
namespace Grindstone {
	namespace GraphicsAPI {
		class Framebuffer;
		class RenderTarget;
		class TextureBindingLayout;
		class GraphicsPipeline;
		class UniformBuffer;
		class VertexArrayObject;
		class VertexBufferObject;
		class IndexBuffer;
	}
}

struct CubemapComponent : public Component {
	CubemapComponent(GameObjectHandle object_handle, ComponentHandle id);
	uint32_t resolution_;
	Grindstone::GraphicsAPI::Texture *cubemap_;
	Grindstone::GraphicsAPI::TextureBinding *cubemap_binding_;
	Grindstone::GraphicsAPI::Framebuffer *capture_fbo_;
	Grindstone::GraphicsAPI::RenderTarget *render_target_;
	float near_;
	float far_;
	enum CaptureMethod : int {
		CAPTURE_REALTIME = 0,
		CAPTURE_BAKE,
		CAPTURE_CUSTOM
	};
	int capture_method_;
	std::string path_;

	REFLECT()
};

class CubemapSystem : public System {
public:
	CubemapSystem();
	void update();

	void loadGraphics();
	void destroyGraphics();

	void bake();

	Grindstone::GraphicsAPI::TextureBindingLayout *texture_binding_layout_;
	Grindstone::GraphicsAPI::Framebuffer *camera_framebuffer_;

	REFLECT_SYSTEM()
private:
	void prepareSphere();
	void prepareUniformBuffer();
	void prepareIrradianceShader();
	void prepareSpecularShader();

	void convoluteIrradiance(CubemapComponent &c);
	void convoluteSpecular(CubemapComponent &c);

	void loadCubemaps();

	Grindstone::GraphicsAPI::TextureSubBinding cube_binding_;
	glm::mat4 projection_;

	Grindstone::GraphicsAPI::GraphicsPipeline *irradiance_pipeline_;
	Grindstone::GraphicsAPI::RenderTarget *irradiance_image_;
	Grindstone::GraphicsAPI::Framebuffer *irradiance_fbo_;

	Grindstone::GraphicsAPI::GraphicsPipeline *specular_pipeline_;
	Grindstone::GraphicsAPI::RenderTarget *specular_image_;
	Grindstone::GraphicsAPI::Framebuffer *specular_fbo_;

	Grindstone::GraphicsAPI::VertexArrayObject *sphere_vao_;
	Grindstone::GraphicsAPI::VertexBuffer *sphere_vbo_;
	Grindstone::GraphicsAPI::IndexBuffer *sphere_ibo_;
	Grindstone::GraphicsAPI::VertexBufferLayout sphere_vertex_layout_;
	Grindstone::GraphicsAPI::RenderTarget *final_buffer_;

	unsigned int total_sphere_indices_;

	Grindstone::GraphicsAPI::UniformBufferBinding *ubb_;
	Grindstone::GraphicsAPI::UniformBuffer *ub_;
	struct ConvolutionBufferObject {
		glm::mat4 matrix_;
		float roughness_;
	} ubo_;
};

class CubemapSubSystem : public SubSystem {
	friend CubemapSystem;
public:
	CubemapSubSystem(Space *space);
	virtual void initialize();
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	CubemapComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	CubemapComponent *getClosestCubemap(glm::vec3);
	Camera *camera_;

	virtual ~CubemapSubSystem();
private:
	std::vector<CubemapComponent> components_;
};

#endif