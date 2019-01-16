#ifndef _POST_COLOR_GRADING_H
#define _POST_COLOR_GRADING_H

#include "Framebuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "BasePost.hpp"

// Post Processing effect used in PostPipelines in CCameras.
// This Post Process Effect takes the assigned Look-Up Table (LUT)
//		and applies it to an image.
class PostProcessColorGrading : public BasePostProcess {
	RenderTargetContainer *target_;
	GraphicsPipeline *gpipeline_;
	Texture *texture_;
	TextureBinding *texture_binding_;
public:
	PostProcessColorGrading(PostPipeline *pipeline, RenderTargetContainer *target);
	~PostProcessColorGrading();
	
	void Process();
};

#endif