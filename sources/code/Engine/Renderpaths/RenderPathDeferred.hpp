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

	Texture *m_cubemap;
	TextureBinding *m_cubemapBinding;

	GraphicsPipeline *m_iblPipeline;
public:
	RenderPathDeferred(GraphicsWrapper *graphics_wrapper_, VertexArrayObject *plane_vao);
	void Draw(Framebuffer *);
};

#endif