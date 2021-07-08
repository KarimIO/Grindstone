#pragma once

#include <glm/glm.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
	}

	void BaseRender(
		GraphicsAPI::Core *core,
		glm::mat4 projectionMatrix,
		glm::mat4 viewMatrix
	);
}
