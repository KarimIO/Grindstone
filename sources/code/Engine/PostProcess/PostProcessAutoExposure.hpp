#ifndef _POST_PROCESS_AUTO_EXPOSURE_HPP
#define _POST_PROCESS_AUTO_EXPOSURE_HPP

#include "BasePost.hpp"

class GraphicsPipeline;
class UniformBuffer;
struct RenderTargetContainer;

class PostProcessAutoExposure : public BasePostProcess {
public:
	PostProcessAutoExposure(PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target);
    virtual void Process();
	float GetExposure();
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

    GraphicsPipeline *gpipeline_;

	RenderTarget *lum_buffer_;
	Framebuffer *lum_framebuffer_;
};

#endif // !POST_PROCESS_AUTO_EXPOSURE_HPP