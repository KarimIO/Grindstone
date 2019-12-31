#ifndef _POST_PROCESS_TONEMAP_HPP
#define _POST_PROCESS_TONEMAP_HPP

#include "BasePost.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct TextureSubBinding;
		class TextureBindingLayout;
		class UniformBufferBinding;
	}
}

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

	Grindstone::GraphicsAPI::TextureSubBinding *tonemap_sub_binding_;
	Grindstone::GraphicsAPI::TextureBindingLayout *tonemap_tbl_;
	Grindstone::GraphicsAPI::UniformBufferBinding *ubb_;

	Grindstone::GraphicsAPI::GraphicsPipeline *gpipeline_;

	struct EffectUBO {
		float vignetteRadius;
		float vignetteSoftness;
		float vignetteStrength;
		float exposure;
		float noiseStrength;
		float time;
	} effect_buffer_;

	bool first_render_ = true;

	Grindstone::GraphicsAPI::UniformBuffer *effect_ub_;
};

#endif // !POST_PROCESS_TONEMAP_HPP