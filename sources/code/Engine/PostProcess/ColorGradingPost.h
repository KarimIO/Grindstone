#ifndef _COLOR_GRADING_POST_H
#define _COLOR_GRADING_POST_H

#include "Framebuffer.h"
#include "Shader.h"

class ColorGradingPost {
	Framebuffer *fbo;
	ShaderProgram *shader;
public:
	void Initialize();
	void Process(Framebuffer *target);
	Framebuffer * GetFramebuffer();
	void Cleanup();
};

#endif