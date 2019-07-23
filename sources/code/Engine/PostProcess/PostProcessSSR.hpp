#ifndef _POST_PROCESS_SSR_HPP
#define _POST_PROCESS_SSR_HPP

#include "BasePost.hpp"

class PostProcessAutoExposure;

class PostProcessSSR : public BasePostProcess {
public:
	PostProcessSSR(PostPipeline *pipeline, RenderTargetContainer *source, RenderTargetContainer *target);
	virtual void Process();
private:
	RenderTargetContainer *source_;
	RenderTargetContainer *target_;
	GraphicsPipeline *gpipeline_;
};

#endif // !POST_PROCESS_TONEMAP_HPP