#ifndef _BASE_POST_H
#define _BASE_POST_H

class GraphicsPipeline;
class UniformBuffer;
class Framebuffer;
class RenderTarget;
class DepthTarget;
class PostPipeline;

struct RenderTargetContainer {
	Framebuffer *framebuffer;
	RenderTarget **render_targets;
	unsigned int num_render_targets;
	DepthTarget *depth_target;
};

// Interface for a PostProcess. Should be created and passed to
//      the PostPipeline in a camera to be used in rendering.
// Each PostProcess should contain all GraphicsPipelines and 
//      Framebuffers necessary to work with other post processing
//      effects and the main rendering as a black box.
// Example:
//      BasePostProcess postProcess;
//      ...
//      postProcess.Process();
class BasePostProcess {
public:
	BasePostProcess(PostPipeline *post);
    virtual void Process() = 0;
	PostPipeline *getPipeline();
protected:
	PostPipeline *pipeline_;
};

#endif