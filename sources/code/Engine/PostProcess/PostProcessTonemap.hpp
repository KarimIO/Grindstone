#ifndef POST_PROCESS_TONEMAP_HPP
#define POST_PROCESS_TONEMAP_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessTonemap : public BasePostProcess {
public:
    PostProcessTonemap(RenderTargetContainer *source, RenderTargetContainer *target);
    virtual void Process();
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

    GraphicsPipeline *pipeline_;

	struct ExposureUBO {
		float exposure;
	} exposure_buffer_;

	UniformBuffer *exposure_ub_;
};

#endif // !POST_PROCESS_TONEMAP_HPP