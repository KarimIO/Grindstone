#pragma once

#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

namespace Grindstone {
	class WorldContext {
	public:
		template <typename T>
		static Grindstone::UniquePtr<Grindstone::WorldContext> Create() {
			return Grindstone::Memory::AllocatorCore::AllocateUnique<T>();
		}

		virtual void SetAsActive() = 0;
	};
}
