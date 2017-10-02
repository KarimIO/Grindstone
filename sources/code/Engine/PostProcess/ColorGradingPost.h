#ifndef _COLOR_GRADING_POST_H
#define _COLOR_GRADING_POST_H

#include "Framebuffer.h"
#include "GraphicsPipeline.h"
#include "BasePost.h"

// Post Processing effect used in PostPipelines in CCameras.
// This Post Process Effect takes the assigned Look-Up Table (LUT)
//		and applies it to an image.
class ColorGradingPost : public BasePostProcess {
	Framebuffer *fbo;
	GraphicsPipeline *shader;
public:
	ColorGradingPost();
	ColorGradingPost(const ColorGradingPost &);
	ColorGradingPost(ColorGradingPost &&);
	~ColorGradingPost();
	
	Framebuffer *Process(Framebuffer *target);
};

#endif