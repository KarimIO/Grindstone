#ifndef _POST_PROCESS_IBL_HPP
#define _POST_PROCESS_IBL_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessIBL : public BasePostProcess {
public:
    PostProcessIBL(PostPipeline *pipeline, RenderTargetContainer *target);
    virtual void Process();
private:
    //RenderTargetContainer *source_;
    RenderTargetContainer *target_;

	TextureBindingLayout  *env_map_;
	TextureSubBinding subbinding_;

    GraphicsPipeline *gpipeline_;
};

#endif // !POST_PROCESS_IBL_HPP