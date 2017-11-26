#ifndef _RENDER_TARGET_H
#define _RENDER_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

struct RenderTargetCreateInfo {
    ImageFormat format;
    uint32_t width, height;
    RenderTargetCreateInfo() {};
    RenderTargetCreateInfo(ImageFormat fmt, uint32_t w, uint32_t h) : format(fmt), width(w), height(h) {}
};

class RenderTarget {
public:
    virtual void Bind() = 0;
    virtual void Bind(uint32_t i) = 0;
    virtual ~RenderTarget() = 0;
};

#endif