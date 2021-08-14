#pragma once

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar;
	}

	void SetupCoreComponents(ECS::ComponentRegistrar* registrar);
}
