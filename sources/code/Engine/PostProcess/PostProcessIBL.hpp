#ifndef _POST_PROCESS_IBL_HPP
#define _POST_PROCESS_IBL_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessSSAO;

class PostProcessIBL : public BasePostProcess {
public:
    PostProcessIBL(PostPipeline *pipeline, RenderTargetContainer *target, int w, int h);
	void prepareIBL();
	void prepareSSAO();
	void ssao();
	void ibl();
    virtual void Process();
	void recreateFramebuffer(unsigned int w, unsigned int h);
	bool usesSSAO();
private:
    //RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	TextureBindingLayout  *env_map_;
	TextureSubBinding subbinding_;

    GraphicsPipeline *gpipeline_;
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

	unsigned int viewport_w_;
	unsigned int viewport_h_;
};

#endif // !POST_PROCESS_IBL_HPP