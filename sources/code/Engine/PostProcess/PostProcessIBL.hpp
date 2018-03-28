#ifndef POST_PROCESS_IBL_HPP
#define POST_PROCESS_IBL_HPP

#include "BasePost.hpp"
#include "GraphicsPipeline.hpp"

class PostProcessIBL : public BasePostProcess {
public:
    PostProcessIBL(RenderTargetContainer *source, RenderTargetContainer *target);
    virtual void Process();
private:
    RenderTargetContainer *source_;
    RenderTargetContainer *target_;

    GraphicsPipeline *pipeline_;
};

#endif // !POST_PROCESS_IBL_HPP