#ifndef _GL_RENDER_TARGET_H
#define _GL_RENDER_TARGET_H

#include <stdint.h>
#include "../GraphicsCommon/RenderTarget.hpp"

class GLRenderTarget : public RenderTarget {
public:
    GLRenderTarget(RenderTargetCreateInfo *cis, uint32_t count);
    virtual uint32_t getHandle();
	virtual uint32_t getHandle(uint32_t i);
    uint32_t getNumRenderTargets();

    float getAverageValue(uint32_t i);

    virtual void Bind();
    virtual void Bind(uint32_t i);
	virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data);
    virtual ~GLRenderTarget();
private:
    uint32_t *handles_;
    uint32_t size_;
	uint32_t width_, height_;
	unsigned int *format_;
};

#endif