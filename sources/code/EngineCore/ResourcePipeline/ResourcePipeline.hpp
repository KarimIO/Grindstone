#pragma once

#include "Uuid.hpp"

namespace Grindstone {
	class ResourcePipeline {
	public:
		std::string GetFilePathOfUuid(Uuid uuid);
	};
}
