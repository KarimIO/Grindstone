#ifndef _RENDER_SPRITE_SYSTEM_H
#define _RENDER_SPRITE_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include "../AssetManagers/AssetReferences.hpp"
#include "../AssetManagers/TextureManager.hpp"

class TextureBinding;
class GraphicsPipeline;
class DepthTarget;

struct SpriteUniformBuffer {
	glm::mat4 model;
	glm::vec4 color;
	float aspect;
	bool render_ortho;
};

struct RenderSpriteComponent : public Component {
	RenderSpriteComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	TextureBinding *texture_binding_;
	TextureHandler texture_handle_;
	glm::vec4 color_;
	float aspect_;
};

class RenderSpriteSystem : public System {
public:
	RenderSpriteSystem();

	void loadDebugSprites();

	GraphicsPipeline *pipeline_;
	TextureBindingLayout *tbl_;
	UniformBuffer *ubo_;
	UniformBufferBinding *ubb_;
	void update(double dt);

	TextureBinding *debug_light_pos_sprite_;
};

class RenderSpriteSubSystem : public SubSystem {
	friend RenderSpriteSystem;
public:
	RenderSpriteSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	RenderSpriteComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderSpriteSubSystem();
	
	void renderSprite(bool render_ortho, float aspect, glm::vec4 color, TextureBinding *binding, glm::mat4 model);
	void renderSprites(bool render_ortho, glm::vec3 cam_pos, DepthTarget *depth_target);
	void handleDebugSprite(bool render_ortho, glm::vec3 color, glm::vec3 cam_pos, GameObjectHandle handle, TextureBinding * tex_binding);
private:
	std::vector<RenderSpriteComponent> components_;
};

#endif