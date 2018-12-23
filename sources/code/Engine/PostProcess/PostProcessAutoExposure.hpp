#if 0
#ifndef POST_PROCESS_AUTO_EXPOSURE_HPP
#define POST_PROCESS_AUTO_EXPOSURE_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessAutoExposure : public BasePostProcess {
public:
	PostProcessAutoExposure(RenderTargetContainer *source, RenderTargetContainer *target);
    virtual void Process();
	float GetExposure();
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

    GraphicsPipeline *pipeline_;

	RenderTarget *lum_buffer_;
	Framebuffer *lum_framebuffer_;
};
#endif
#endif // !POST_PROCESS_AUTO_EXPOSURE_HPP