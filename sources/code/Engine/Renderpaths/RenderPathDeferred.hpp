#ifndef _RENDERPATH_DEFERRED_H
#define _RENDERPATH_DEFERRED_H

#include "RenderPath.h"
#include "../Systems/Terrain.h"
#include "Framebuffer.h"
#include "GraphicsPipeline.h"
#include "../Systems/Terrain.h"
#include "Systems/SLight.h"

class RenderPathDeferred : public RenderPath {
	GraphicsWrapper *m_graphics_wrapper_;

	VertexArrayObject *planeVAO;
	VertexBuffer *planeVBO;

	UniformBuffer *deffUBO;
	DefferedUBO deffUBOBuffer;

	Texture *m_cubemap;
	TextureBinding *m_cubemapBinding;

	GraphicsPipeline *m_iblPipeline;
public:
	RenderPathDeferred(GraphicsWrapper *graphics_wrapper_);
	void Draw(Framebuffer *);
};

#endif