#pragma once

#include "Framebuffer.hpp"
#include "Formats.hpp"
#include <stdint.h>

typedef union ClearColor {
	float       float32[4];
	int32_t     int32[4];
	uint32_t    uint32[4];
} ClearColorValue;

struct ClearDepthStencil {
	bool hasDepthStencilAttachment;
	float       depth;
	uint32_t    stencil;
};

struct RenderPassCreateInfo {
	uint32_t m_width;
	uint32_t m_height;
	ClearColorValue *m_colorClearValues;
	uint32_t m_colorClearCount;
	ClearDepthStencil m_depthStencilClearValue;
	DepthFormat m_depthFormat;
};

class RenderPass {
public:
	virtual ~RenderPass() {};
};