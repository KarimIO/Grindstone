#pragma once

#include <map>
#include <vector>
#include <Common/HashedString.hpp>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>

#include "WorldContextSet.hpp"
#include "WorldContext.hpp"

namespace Grindstone {
	class WorldContextManager {
	public:
		static WorldContextManager* GetInstance();

		virtual void Register(HashedString name, Grindstone::UniquePtr<Grindstone::WorldContext>(*factoryFn)());
		virtual void Unregister(HashedString name);
		virtual void SetActiveWorldContextSet(Grindstone::WorldContextSet*);
		[[nodiscard]] virtual Grindstone::WorldContextSet* GetActiveWorldContextSet();
		[[nodiscard]] virtual Grindstone::WorldContextSet& Create();
		virtual void Remove(Grindstone::WorldContextSet& cxtSet);

	private:
		size_t activeWorldIndex;
		std::map<HashedString, Grindstone::UniquePtr<Grindstone::WorldContext> (*)()> factoryFunctions;
		std::vector<Grindstone::WorldContextSet> worldContextSets;
	};
}
