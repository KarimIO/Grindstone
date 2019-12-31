#pragma once

#include <stdint.h>
#include "../GraphicsCommon/RenderTarget.hpp"

class VulkanRenderTarget : public RenderTarget {
public:
	VulkanRenderTarget(RenderTargetCreateInfo *cis, uint32_t count, bool cubemap);
    virtual uint32_t getHandle();
	virtual uint32_t getHandle(uint32_t i);
    uint32_t getNumRenderTargets();

    float getAverageValue(uint32_t i);

    virtual void Bind();
    virtual void Bind(uint32_t i);
	virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data);
    virtual ~VulkanRenderTarget();
	bool is_cubemap_;
private:
    uint32_t *handles_;
    uint32_t size_;
	uint32_t width_, height_;
	unsigned int *format_;
};