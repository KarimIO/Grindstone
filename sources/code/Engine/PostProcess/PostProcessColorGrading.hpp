#ifndef _POST_COLOR_GRADING_H
#define _POST_COLOR_GRADING_H

#include "BasePost.hpp"

class Framebuffer;
class GraphicsPipeline;
class Texture;
class TextureBinding;
class TextureBindingLayout;

// Post Processing effect used in PostPipelines in CCameras.
// This Post Process Effect takes the assigned Look-Up Table (LUT)
//		and applies it to an image.
class PostProcessColorGrading : public BasePostProcess {
	RenderTargetContainer *target_;
	Framebuffer **target_fbo_;
	GraphicsPipeline *gpipeline_;
	Texture *texture_;
	TextureBinding *texture_binding_;
	TextureBindingLayout *grading_tbl_;
public:
	PostProcessColorGrading(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *target, Framebuffer **target_fbo);
	virtual ~PostProcessColorGrading() override;
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
	
	void Process();
};

#endif