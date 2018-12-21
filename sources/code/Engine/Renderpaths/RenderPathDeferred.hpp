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

class RenderPathDeferred : public RenderPath {
public:
	RenderPathDeferred();
	void render(Framebuffer *default, glm::mat4 p, glm::mat4 v, glm::vec3 eye);
	void renderLights();
	void createPointLightShader();
private:
	void createFramebuffer();
	TextureBindingLayout *gbuffer_tbl_;
	Framebuffer *gbuffer_;
	RenderTarget *render_targets_;
	DepthTarget *depth_target_;

	std::vector<TextureSubBinding> bindings;

	UniformBufferBinding *point_light_ubb_;
	UniformBufferBinding *deff_ubb_;

	UniformBuffer *point_light_ubo_handler_;
	UniformBuffer *deff_ubo_handler_;

	VertexArrayObject *plane_vao_;
	VertexBuffer *plane_vbo_;
	VertexBindingDescription plane_vbd_;
	VertexAttributeDescription plane_vad_;

	struct LightPointUBO {
		float position[3];
		float attenuationRadius;
		float color[3];
		float power;
		bool shadow;
	} light_point_ubo_;

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