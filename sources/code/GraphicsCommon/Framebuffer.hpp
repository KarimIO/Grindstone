#pragma once

#include "RenderPass.hpp"
#include "Formats.hpp"
#include "RenderTarget.hpp"
#include <vector>
#include <stdint.h>

class RenderPass;

struct DefaultFramebufferCreateInfo {
	uint32_t width;
	uint32_t height;
	RenderPass *renderPass;
};

struct FramebufferCreateInfo {
	RenderPass *render_pass;

	RenderTarget **render_target_lists;
	uint32_t num_render_target_lists;
	RenderTarget *depth_target;
};


class Framebuffer {
public:
	virtual void Clear() = 0;
	virtual void CopyFrom(Framebuffer *) = 0;
	virtual void Bind() = 0;
	virtual void BindWrite() = 0;
	virtual void BindRead() = 0;
	virtual void Unbind() = 0;
	virtual ~Framebuffer() = 0;
};