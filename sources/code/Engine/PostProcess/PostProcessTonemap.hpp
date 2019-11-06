#ifndef _POST_PROCESS_TONEMAP_HPP
#define _POST_PROCESS_TONEMAP_HPP

#include "BasePost.hpp"

struct TextureSubBinding;
class TextureBindingLayout;
class UniformBufferBinding;

class PostProcessAutoExposure;

class PostProcessTonemap : public BasePostProcess {
public:
    PostProcessTonemap(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure);
    virtual void Process();
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	PostProcessAutoExposure *auto_exposure_;

	TextureSubBinding *tonemap_sub_binding_;
	TextureBindingLayout *tonemap_tbl_;
	UniformBufferBinding *ubb_;

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