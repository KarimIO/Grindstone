#ifndef _LOADING_SCREEN_H
#define _LOADING_SCREEN_H

#include "../GraphicsCommon/GraphicsWrapper.hpp"

class LoadingScreen {
public:
	LoadingScreen(GraphicsWrapper *gw);
	void Render(double dt);
	~LoadingScreen();
private:
	struct LoadUBO {
		float aspect;
		float time;
		float buffer[2];
	} loadUBO;
	GraphicsWrapper *graphics_wrapper_;
	Texture *texture_;
	GraphicsPipeline *pipeline_;
	RenderPass *render_pass_;
	VertexArrayObject *vao_;
	VertexBuffer *vbo_;
	TextureBinding *tb_;
	TextureBindingLayout *tbl_;
	UniformBufferBinding *ubb_;
	UniformBuffer *ubo_;
};

#endif