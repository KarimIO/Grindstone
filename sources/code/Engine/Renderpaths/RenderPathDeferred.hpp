#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.hpp"
#include <VertexBuffer.hpp>
#include <Texture.hpp>

namespace Grindstone  {
	namespace GraphicsAPI {
		class RenderTarget;
		class DepthTarget;
		class GraphicsPipeline;
		class UniformBufferBinding;
		class UniformBuffer;
		class TextureBindingLayout;
		class VertexArrayObject;
		class VertexBuffer;
	}
}

class Scene;

class RenderPathDeferred : public RenderPath {
public:
	bool filled_;
	bool wireframe_;

	void createShadowTextureBindingLayout();

	RenderPathDeferred(unsigned int w, unsigned int h);
	virtual void setDebugMode(unsigned int d) override;
	virtual unsigned int getDebugMode() override;
	void render(Grindstone::GraphicsAPI::Framebuffer *default_fb, Grindstone::GraphicsAPI::DepthTarget *depthTarget, Space *scene);
	void renderLights(Grindstone::GraphicsAPI::Framebuffer *fbo, Space *scene);
	virtual void recreateFramebuffer(unsigned int w, unsigned int h) override;
	virtual void destroyGraphics() override;
	virtual void reloadGraphics() override;
	void destroyFramebuffers();
private:
	void createPointLightShader();
	void createSpotLightShader();
	void createDirectionalLightShader();
	void createDebugShader();
	void renderDebug(Grindstone::GraphicsAPI::Framebuffer *fbo);
private:
	void CreateFramebuffer(unsigned int width, unsigned int height);
	Grindstone::GraphicsAPI::Framebuffer *gbuffer_;
	Grindstone::GraphicsAPI::RenderTarget *render_targets_;
	Grindstone::GraphicsAPI::DepthTarget *depth_target_;

	Grindstone::GraphicsAPI::TextureSubBinding shadow_binding_;
	Grindstone::GraphicsAPI::TextureBindingLayout *shadow_tbl_;

	Grindstone::GraphicsAPI::UniformBufferBinding *point_light_ubb_;
	Grindstone::GraphicsAPI::UniformBufferBinding *spot_light_ubb_;
	Grindstone::GraphicsAPI::UniformBufferBinding *directional_light_ubb_;
	Grindstone::GraphicsAPI::UniformBufferBinding *debug_ubb_;

	Grindstone::GraphicsAPI::UniformBuffer *point_light_ubo_handler_;
	Grindstone::GraphicsAPI::UniformBuffer *spot_light_ubo_handler_;
	Grindstone::GraphicsAPI::UniformBuffer *directional_light_ubo_handler_;
	Grindstone::GraphicsAPI::UniformBuffer *debug_ubo_handler_;

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

	unsigned int debug_mode_;

	Grindstone::GraphicsAPI::GraphicsPipeline *spot_light_pipeline_;
	Grindstone::GraphicsAPI::GraphicsPipeline *point_light_pipeline_;
	Grindstone::GraphicsAPI::GraphicsPipeline *directional_light_pipeline_;
	Grindstone::GraphicsAPI::GraphicsPipeline *debug_pipeline_;
};

#endif