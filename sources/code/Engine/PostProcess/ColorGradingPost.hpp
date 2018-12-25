#ifndef _COLOR_GRADING_POST_H
#define _COLOR_GRADING_POST_H

#include "Framebuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "BasePost.hpp"

// Post Processing effect used in PostPipelines in CCameras.
// This Post Process Effect takes the assigned Look-Up Table (LUT)
//		and applies it to an image.
class ColorGradingPost : public BasePostProcess {
	Framebuffer *fbo;
	GraphicsPipeline *shader;
public:
	ColorGradingPost(PostPipeline *pipeline);
	~ColorGradingPost();
	
	Framebuffer *Process(Framebuffer *target);
};

#endif