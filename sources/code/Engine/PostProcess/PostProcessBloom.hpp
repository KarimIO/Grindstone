#ifndef _POST_PROCESS_BLOOM_HPP
#define _POST_PROCESS_BLOOM_HPP

#include "BasePost.hpp"

class GraphicsPipeline;
class UniformBuffer;
struct RenderTargetContainer;
class PostProcessAutoExposure;

class PostProcessBloom : public BasePostProcess {
public:
	PostProcessBloom(PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure);
    virtual void Process();
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	PostProcessAutoExposure *auto_exposure_;

    GraphicsPipeline *gpipeline_;
};

#endif // !POST_PROCESS_BLOOM_HPP