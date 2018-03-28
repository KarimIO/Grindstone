#ifndef POST_PROCESS_SSAO_HPP
#define POST_PROCESS_SSAO_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessSSAO : public BasePostProcess {
public:
    PostProcessSSAO(RenderTargetContainer *source);
    virtual void Process();
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
};

#endif // !POST_PROCESS_SSAO_HPP