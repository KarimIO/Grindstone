#ifndef _POST_COLOR_GRADING_H
#define _POST_COLOR_GRADING_H

#include "BasePost.hpp"

class Framebuffer;
class GraphicsPipeline;
class Texture;
class TextureBinding;

// Post Processing effect used in PostPipelines in CCameras.
// This Post Process Effect takes the assigned Look-Up Table (LUT)
//		and applies it to an image.
class PostProcessColorGrading : public BasePostProcess {
	RenderTargetContainer *target_;
	Framebuffer **target_fbo_;
	GraphicsPipeline *gpipeline_;
	Texture *texture_;
	TextureBinding *texture_binding_;
public:
	PostProcessColorGrading(PostPipeline *pipeline, RenderTargetContainer *target, Framebuffer **target_fbo);
	~PostProcessColorGrading();
	
	void Process();
};

#endif