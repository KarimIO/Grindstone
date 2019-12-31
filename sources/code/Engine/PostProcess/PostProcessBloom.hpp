#ifndef _POST_PROCESS_BLOOM_HPP
#define _POST_PROCESS_BLOOM_HPP

#include "BasePost.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsPipeline;
		class UniformBuffer;
	}
}

class PostProcessAutoExposure;
struct RenderTargetContainer;

class PostProcessBloom : public BasePostProcess {
public:
	PostProcessBloom(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target, PostProcessAutoExposure *auto_exposure);
	virtual ~PostProcessBloom() override;
    virtual void Process();
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	PostProcessAutoExposure *auto_exposure_;

	Grindstone::GraphicsAPI::GraphicsPipeline *gpipeline_;
};

#endif // !POST_PROCESS_BLOOM_HPP