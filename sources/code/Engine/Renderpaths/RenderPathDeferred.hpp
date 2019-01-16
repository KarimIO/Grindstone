#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.hpp"
#include <VertexBuffer.hpp>
#include <Texture.hpp>

class RenderTarget;
class DepthTarget;
class GraphicsPipeline;
class UniformBufferBinding;
class UniformBuffer;
class TextureBindingLayout;
class VertexArrayObject;
class VertexBuffer;
class Scene;

class RenderPathDeferred : public RenderPath {
public:
	RenderPathDeferred();
	void render(Framebuffer *default, Space *scene);
	void renderLights(Framebuffer *fbo, Space *scene);
	void createPointLightShader();
	void createSpotLightShader();
	void createDirectionalLightShader();
private:
	void createFramebuffer();
	Framebuffer *gbuffer_;
	RenderTarget *render_targets_;
	DepthTarget *depth_target_;

	TextureSubBinding shadow_binding_;
	TextureBindingLayout *shadow_tbl_;

	UniformBufferBinding *point_light_ubb_;
	UniformBufferBinding *spot_light_ubb_;
	UniformBufferBinding *directional_light_ubb_;

	UniformBuffer *point_light_ubo_handler_;
	UniformBuffer *spot_light_ubo_handler_;
	UniformBuffer *directional_light_ubo_handler_;

	struct LightPointUBO {
		glm::vec3 position;
		float attenuationRadius;
		glm::vec3 color;
		float power;
		bool shadow;
	} light_point_ubo_;

	struct LightSpotUBO {
		glm::mat4 shadow_mat;
		glm::vec3 position;
		float attenuationRadius;
		glm::vec3 color;
		float power;
		glm::vec3 direction;
		float innerAngle;
		float outerAngle;
		float shadow_resolution;
		bool shadow;
	} light_spot_ubo_;

	struct LightDirectionalUBO {
		glm::mat4 shadow_mat;
		glm::vec3 direction;
		float source_radius;
		glm::vec3 color;
		float power;
		float shadow_resolution;
		bool shadow;
	} light_directional_ubo_;

	GraphicsPipeline *spot_light_pipeline_;
	GraphicsPipeline *point_light_pipeline_;
	GraphicsPipeline *directional_light_pipeline_;
};

#endif