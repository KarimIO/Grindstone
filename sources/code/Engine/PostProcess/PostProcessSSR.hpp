#ifndef _POST_PROCESS_SSR_HPP
#define _POST_PROCESS_SSR_HPP

#include "BasePost.hpp"

class PostProcessAutoExposure;

class PostProcessSSR : public BasePostProcess {
public:
	PostProcessSSR(unsigned int w, unsigned h, PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target);
	virtual ~PostProcessSSR() override;
	virtual void Process();
	virtual void resizeBuffers(unsigned int w, unsigned h) override;
	virtual void reloadGraphics(unsigned int w, unsigned h) override;
	virtual void destroyGraphics() override;
private:
	RenderTargetContainer *source_;
	RenderTargetContainer *target_;
	GraphicsPipeline *gpipeline_;
};

#endif // !POST_PROCESS_TONEMAP_HPP