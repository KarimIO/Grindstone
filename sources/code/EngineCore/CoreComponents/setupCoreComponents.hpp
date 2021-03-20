#pragma once

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar;
	}

	void setupCoreComponents(ECS::ComponentRegistrar* registrar);
}
