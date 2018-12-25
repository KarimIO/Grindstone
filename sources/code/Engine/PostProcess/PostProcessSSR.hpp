#ifndef POST_PROCESS_SSR_HPP
#define POST_PROCESS_SSR_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessSSR : public BasePostProcess {
public:
	PostProcessSSR(BasePostProcess *pipeline, RenderTargetContainer *source, RenderTargetContainer *target);

    virtual void Process();
private:
	RenderTargetContainer *source_;
	RenderTargetContainer *target_;

    GraphicsPipeline *pipeline_;

	Texture *SSR_noise_;
	TextureBinding *SSR_noise_binding_;

	struct SSRBufferObject {
		float kernel[32 * 4];
		float radius;
    	float bias;
	} SSR_buffer;

	UniformBuffer *SSR_ub;
};

#endif // !POST_PROCESS_SSR_HPP