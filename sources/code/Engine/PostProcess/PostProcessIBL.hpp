#ifndef _POST_PROCESS_IBL_HPP
#define _POST_PROCESS_IBL_HPP

#include "BasePost.hpp"
#include <GraphicsCommon/GraphicsPipeline.hpp>

class PostProcessSSAO;

class PostProcessIBL : public BasePostProcess {
public:
    PostProcessIBL(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *target);
	virtual ~PostProcessIBL() override;
	void prepareIBL(unsigned int w, unsigned h);
	void prepareSSAO(unsigned int w, unsigned h);
	void ssao();
	void ibl();
    virtual void Process();
	void recreateFramebuffer(unsigned int w, unsigned int h);
	bool usesSSAO();
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
private:
    //RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	Grindstone::GraphicsAPI::TextureBindingLayout  *env_map_;
	Grindstone::GraphicsAPI::TextureSubBinding subbinding_;

	Grindstone::GraphicsAPI::GraphicsPipeline *gpipeline_;
private:
	RenderTargetContainer *source_;

	Grindstone::GraphicsAPI::GraphicsPipeline *pipeline_;

	Grindstone::GraphicsAPI::Texture *ssao_noise_;
	Grindstone::GraphicsAPI::TextureBinding *ssao_noise_binding_;

	struct SSAOBufferObject {
		float kernel[32 * 4];
		float radius;
		float bias;
	} ssao_buffer;

	Grindstone::GraphicsAPI::UniformBuffer *ssao_ub;

	Grindstone::GraphicsAPI::TextureSubBinding ssao_output_;
	Grindstone::GraphicsAPI::RenderTarget *ssao_buffer_;
	Grindstone::GraphicsAPI::Framebuffer *ssao_fbo_;
	Grindstone::GraphicsAPI::TextureBindingLayout *ssao_layout_;

	unsigned int viewport_w_;
	unsigned int viewport_h_;
};

#endif // !POST_PROCESS_IBL_HPP