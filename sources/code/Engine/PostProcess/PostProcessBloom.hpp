#ifndef _POST_PROCESS_BLOOM_HPP
#define _POST_PROCESS_BLOOM_HPP

#include "BasePost.hpp"

class GraphicsPipeline;
class UniformBuffer;
struct RenderTargetContainer;
class PostProcessAutoExposure;

class PostProcessBloom : public BasePostProcess {
public:
	PostProcessBloom(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure);
	virtual ~PostProcessBloom() override;
    virtual void Process();
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	PostProcessAutoExposure *auto_exposure_;

    GraphicsPipeline *gpipeline_;
};

#endif // !POST_PROCESS_BLOOM_HPP