#pragma once

namespace Grindstone {
	struct TransformComponent {
		float position[3];
		float angles[3];
		float scale[3];

		float world[4][4];
	};
}
