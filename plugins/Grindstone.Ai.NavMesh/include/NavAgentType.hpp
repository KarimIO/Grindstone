#pragma once

#include <stdint.h>

namespace Grindstone::Ai {
	using NavAgentTypeId = uint8_t;

	struct NavAgentType {
		float radius;
		float height;
		float stepHeight;
		float maxSlope; // angle
		float dropHeight; // What height an agent can jump down to
		float jumpDistance; // Jumping between polys
	};
}
