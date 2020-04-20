#ifndef _IRR_VOL_SYSTEM_H
#define _IRR_VOL_SYSTEM_H

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

struct IrradianceVolumeComponent : public Component {
	IrradianceVolumeComponent(GameObjectHandle object_handle, ComponentHandle id);

	REFLECT()
};

class IrradianceVolumeSystem : public System {
public:
	IrradianceVolumeSystem();
	void update();

	void loadGraphics();
	void destroyGraphics();

	Grindstone::GraphicsAPI::TextureBindingLayout *texture_binding_layout_;
	Grindstone::GraphicsAPI::Framebuffer *camera_framebuffer_;

	REFLECT_SYSTEM()
private:
	void prepareCube();
	void prepareUniformBuffer();
	void prepareIrradianceShader();

	void loadIrradianceVolumes();

	Grindstone::GraphicsAPI::TextureSubBinding cube_binding_;
	glm::mat4 projection_;

	Grindstone::GraphicsAPI::GraphicsPipeline *irradiance_pipeline_;

	Grindstone::GraphicsAPI::VertexArrayObject *cube_vao_;
	Grindstone::GraphicsAPI::VertexBuffer *cube_vbo_;
	Grindstone::GraphicsAPI::IndexBuffer *cube_ibo_;
	Grindstone::GraphicsAPI::VertexBufferLayout cube_vertex_layout_;
	Grindstone::GraphicsAPI::RenderTarget *final_buffer_;

	Grindstone::GraphicsAPI::UniformBufferBinding *ubb_;
	Grindstone::GraphicsAPI::UniformBuffer *ub_;
	struct ConvolutionBufferObject {
		glm::mat4 matrix_;
		float roughness_;
	} ubo_;
};

class IrradianceVolumeSubSystem : public SubSystem {
	friend IrradianceVolumeSystem;
public:
	IrradianceVolumeSubSystem(Space *space);
	virtual void initialize();
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	IrradianceVolumeComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	Camera *camera_;

	virtual ~IrradianceVolumeSubSystem();
private:
	std::vector<IrradianceVolumeComponent> components_;
};

#endif