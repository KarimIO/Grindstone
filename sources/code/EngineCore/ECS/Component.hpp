#pragma once

#include <cinttypes>

namespace Grindstone {
	namespace ECS {
		using ComponentType = std::uint8_t;
		const ComponentType MAX_COMPONENTS = UINT8_MAX;
	}
}

#define COMPONENT_DEFINE(T) template class __declspec(dllimport) Grindstone::ECS::ComponentArray<T>; \
							Grindstone::ECS::ComponentType Grindstone::ECS::ComponentArray<T>::type_id_ = 0;