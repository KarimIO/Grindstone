#ifndef _DEPTH_TARGET_H
#define _DEPTH_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

struct DepthTargetCreateInfo {
    ImageFormat format;
    uint32_t width, height;
	DepthTargetCreateInfo() {};
	DepthTargetCreateInfo(ImageFormat fmt, uint32_t w, uint32_t h) : format(fmt), width(w), height(h) {}
};

class DepthTarget {
public:
    virtual void Bind() = 0;
    virtual void Bind(uint32_t i) = 0;
    virtual ~DepthTarget() = 0;
};

#endif