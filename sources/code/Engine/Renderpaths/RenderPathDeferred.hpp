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
	void render(Framebuffer *default, Space *scene, glm::mat4 p, glm::mat4 v, glm::vec3 eye);
	void renderLights(Framebuffer *fbo, Space *scene);
	void createPointLightShader();
	void createSpotLightShader();
	void createDirectionalLightShader();
private:
	void createFramebuffer();
	TextureBindingLayout *gbuffer_tbl_;
	Framebuffer *gbuffer_;
	RenderTarget *render_targets_;
	DepthTarget *depth_target_;

	std::vector<TextureSubBinding> bindings;

	UniformBufferBinding *point_light_ubb_;
	UniformBufferBinding *spot_light_ubb_;
	UniformBufferBinding *directional_light_ubb_;
	UniformBufferBinding *deff_ubb_;

	UniformBuffer *point_light_ubo_handler_;
	UniformBuffer *spot_light_ubo_handler_;
	UniformBuffer *directional_light_ubo_handler_;
	UniformBuffer *deff_ubo_handler_;

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
		bool shadow;
	} light_spot_ubo_;

	struct LightDirectionalUBO {
		glm::mat4 shadow_mat;
		glm::vec3 direction;
		float source_radius;
		glm::vec3 color;
		float power;
		bool shadow;
	} light_directional_ubo_;

	struct DefferedUBO {
		glm::mat4 view;
		glm::mat4 invProj;
		glm::vec4 eyePos;
		glm::vec4 resolution;
		float time;
	} deferred_ubo_;

	GraphicsPipeline *spot_light_pipeline_;
	GraphicsPipeline *point_light_pipeline_;
	GraphicsPipeline *directional_light_pipeline_;
};

#endif