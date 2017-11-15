#pragma once

#include "RenderPass.hpp"
#include "Formats.hpp"
#include <vector>
#include <stdint.h>

class RenderPass;

struct DefaultFramebufferCreateInfo {
	uint32_t width;
	uint32_t height;
	RenderPass *renderPass;

	DepthFormat depthFormat = FORMAT_DEPTH_32;
};

struct FramebufferCreateInfo {
	uint32_t width;
	uint32_t height;
	RenderPass *renderPass;

	ColorFormat *colorFormats;
	uint32_t numColorTargets;
	DepthFormat depthFormat;
};


class Framebuffer {
public:
	virtual void Clear() = 0;
	virtual void Blit(int i, int x, int y, int w, int h) = 0;
	virtual void BindWrite() = 0;
	virtual void BindRead() = 0;
	virtual void BindTextures() = 0;
	virtual void Unbind() = 0;
	virtual ~Framebuffer() {};
};