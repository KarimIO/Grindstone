#ifndef _POST_PROCESS_TONEMAP_HPP
#define _POST_PROCESS_TONEMAP_HPP

#include "BasePost.hpp"

class PostProcessAutoExposure;

class PostProcessTonemap : public BasePostProcess {
public:
    PostProcessTonemap(PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure);
    virtual void Process();
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	PostProcessAutoExposure *auto_exposure_;

    GraphicsPipeline *gpipeline_;

	struct EffectUBO {
		float vignetteRadius;
		float vignetteSoftness;
		float vignetteStrength;
		float exposure;
		float noiseStrength;
		float time;
	} effect_buffer_;

	bool first_render_ = true;

	UniformBuffer *effect_ub_;
};

#endif // !POST_PROCESS_TONEMAP_HPP