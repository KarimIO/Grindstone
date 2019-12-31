#ifndef _BASE_POST_H
#define _BASE_POST_H

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsPipeline;
		class UniformBuffer;
		class Framebuffer;
		class RenderTarget;
		class DepthTarget;
	}
}

class PostPipeline;

struct RenderTargetContainer {
	Grindstone::GraphicsAPI::Framebuffer *framebuffer;
	Grindstone::GraphicsAPI::RenderTarget **render_targets;
	unsigned int num_render_targets;
	Grindstone::GraphicsAPI::DepthTarget *depth_target;
};

// Interface for a PostProcess. Should be created and passed to
//      the PostPipeline in a camera to be used in rendering.
// Each PostProcess should contain all GraphicsPipelines and 
//      Grindstone::GraphicsAPI::Framebuffers necessary to work with other post processing
//      effects and the main rendering as a black box.
// Example:
//      BasePostProcess postProcess;
//      ...
//      postProcess.Process();
class BasePostProcess {
public:
	BasePostProcess(PostPipeline *post);
	virtual ~BasePostProcess();
    virtual void Process() = 0;
	virtual void resizeBuffers(unsigned int w, unsigned h);
	virtual void reloadGraphics(unsigned int w, unsigned h);
	virtual void destroyGraphics();
	PostPipeline *getPipeline();
protected:
	PostPipeline *pipeline_;
};

#endif