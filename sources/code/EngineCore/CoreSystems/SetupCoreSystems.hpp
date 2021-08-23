#pragma once

namespace Grindstone {
	namespace ECS {
		class SystemRegistrar;
	}
	
	void SetupCoreSystems(ECS::SystemRegistrar* registrar);
}
