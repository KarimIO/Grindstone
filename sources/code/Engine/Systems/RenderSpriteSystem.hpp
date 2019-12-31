#ifndef _RENDER_SPRITE_SYSTEM_H
#define _RENDER_SPRITE_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "../AssetManagers/AssetReferences.hpp"
#include "../AssetManagers/TextureManager.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class TextureBinding;
		class GraphicsPipeline;
		class DepthTarget;
	}
}

struct SpriteUniformBuffer {
	glm::mat4 model;
	glm::vec4 color;
	float aspect;
	bool render_ortho;
};

struct RenderSpriteComponent : public Component {
	RenderSpriteComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	Grindstone::GraphicsAPI::TextureBinding *texture_binding_;
	TextureHandler texture_handle_;
	glm::vec4 color_;
	float aspect_;
};

class RenderSpriteSystem : public System {
public:
	RenderSpriteSystem();

	void loadDebugSprites();

	Grindstone::GraphicsAPI::GraphicsPipeline *pipeline_;
	Grindstone::GraphicsAPI::TextureBindingLayout *tbl_;
	Grindstone::GraphicsAPI::UniformBuffer *ubo_;
	Grindstone::GraphicsAPI::UniformBufferBinding *ubb_;
	void update();

	Grindstone::GraphicsAPI::TextureBinding *debug_light_pos_sprite_;
};

class RenderSpriteSubSystem : public SubSystem {
	friend RenderSpriteSystem;
public:
	RenderSpriteSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	RenderSpriteComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderSpriteSubSystem();
	
	void renderSprite(bool render_ortho, float aspect, glm::vec4 color, Grindstone::GraphicsAPI::TextureBinding *binding, glm::mat4 model);
	void renderSprites(bool render_ortho, glm::vec3 cam_pos, Grindstone::GraphicsAPI::DepthTarget *depth_target);
	void handleDebugSprite(bool render_ortho, glm::vec3 color, glm::vec3 cam_pos, GameObjectHandle handle, Grindstone::GraphicsAPI::TextureBinding * tex_binding);
private:
	std::vector<RenderSpriteComponent> components_;
};

#endif