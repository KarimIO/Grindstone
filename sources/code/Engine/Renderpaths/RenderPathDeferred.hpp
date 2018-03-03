#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.hpp"
#include "../Systems/SGeometryTerrain.hpp"
#include "Framebuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "Systems/SLight.hpp"

class RenderPathDeferred : public RenderPath {
	GraphicsWrapper *m_graphics_wrapper_;

	VertexArrayObject *plane_vao_;

	Texture *ssao_noise_;
	TextureBinding *ssao_noise_binding_;
	GraphicsPipeline *ssao_pipeline_;

	GraphicsPipeline *m_iblPipeline;

	struct SSAOBufferObject {
		float kernel[32 * 4];
		float radius;
    	float bias;
	} ssao_buffer;

	UniformBuffer *ssao_ub;
public:
	RenderPathDeferred(GraphicsWrapper *graphics_wrapper_, VertexArrayObject *plane_vao);
	void Draw(Framebuffer *);
};

#endif