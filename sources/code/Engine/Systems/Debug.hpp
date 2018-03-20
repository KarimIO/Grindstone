#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "../GraphicsCommon/Framebuffer.hpp"
#include "../GraphicsCommon/GraphicsPipeline.hpp"

class Debug {
private:
    Framebuffer *framebuffer_;
    GraphicsPipeline *pipeline_;
    UniformBuffer *ubo;
    unsigned int debug_mode_;
public:
    Debug();
    const unsigned int GetDebugMode();
    void SetInitialize(Framebuffer *framebuffer);
    void SwitchDebug(double p);
    void Draw();
    ~Debug();
};

#endif