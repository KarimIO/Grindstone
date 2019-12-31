#ifndef _GL_DEPTH_TARGET_H
#define _GL_DEPTH_TARGET_H

#include <stdint.h>
#include "../GraphicsCommon/DepthTarget.hpp"

class VulkanDepthTarget : public DepthTarget {
public:
	VulkanDepthTarget(DepthTargetCreateInfo cis);
    uint32_t getHandle();
	bool cubemap;

    virtual void BindFace(int k);
    virtual void Bind(int i);
    virtual ~VulkanDepthTarget();
private:
    uint32_t handle_;
};

#endif