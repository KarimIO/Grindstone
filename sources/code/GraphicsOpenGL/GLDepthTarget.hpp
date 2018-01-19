#ifndef _GL_DEPTH_TARGET_H
#define _GL_DEPTH_TARGET_H

#include <stdint.h>
#include "../GraphicsCommon/DepthTarget.hpp"

class GLDepthTarget : public DepthTarget {
public:
    GLDepthTarget(DepthTargetCreateInfo *cis, uint32_t count);
    uint32_t getHandle();
    uint32_t getHandle(uint32_t i);
    uint32_t getNumDepthTargets();

    virtual void Bind();
    virtual void Bind(uint32_t i);
    virtual ~GLDepthTarget();
private:
    uint32_t *handles_;
    uint32_t size_;
};

#endif