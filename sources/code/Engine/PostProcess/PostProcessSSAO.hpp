#ifndef _POST_PROCESS_SSAO_HPP
#define _POST_PROCESS_SSAO_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessSSAO : public BasePostProcess {
public:
    PostProcessSSAO(PostPipeline *pipeline, RenderTargetContainer *source);
    virtual void Process();
	TextureBindingLayout *getLayout();
	Framebuffer *getFramebuffer();
private:
    RenderTargetContainer *source_;

    GraphicsPipeline *pipeline_;

	Texture *ssao_noise_;
	TextureBinding *ssao_noise_binding_;

	struct SSAOBufferObject {
		float kernel[32 * 4];
		float radius;
    	float bias;
	} ssao_buffer;

	UniformBuffer *ssao_ub;

	TextureSubBinding ssao_output_;
	RenderTarget *ssao_buffer_;
	Framebuffer *ssao_fbo_;
	TextureBindingLayout *ssao_layout_;
};

#endif // !POST_PROCESS_SSAO_HPP