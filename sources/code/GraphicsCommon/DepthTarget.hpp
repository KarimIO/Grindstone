#ifndef _DEPTH_TARGET_H
#define _DEPTH_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

struct DepthTargetCreateInfo {
    DepthFormat format;
    uint32_t width, height;
	bool shadow_map;
	DepthTargetCreateInfo() {};
	DepthTargetCreateInfo(DepthFormat fmt, uint32_t w, uint32_t h, bool s) : format(fmt), width(w), height(h), shadow_map(s) {}
};

class DepthTarget {
public:
	virtual ~DepthTarget() {};
};

#endif