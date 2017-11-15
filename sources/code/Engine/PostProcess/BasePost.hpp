#ifndef _BASE_POST_H
#define _BASE_POST_H

#include "../../GraphicsCommon/Framebuffer.hpp"

// Interface for a PostProcess. Should be created and passed to
//      the PostPipeline in a camera to be used in rendering.
// Each PostProcess should contain all GraphicsPipelines and 
//      Framebuffers necessary to work with other post processing
//      effects and the main rendering as a black box.
// Example:
//      BasePostProcess postProcess;
//      ...
//      fbo = postProcess.Process(fbo);
class BasePostProcess {
public:
    virtual Framebuffer *Process(Framebuffer *fbo) = 0;
};

#endif