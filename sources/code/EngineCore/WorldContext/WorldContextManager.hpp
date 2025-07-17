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
		[[nodiscard]] virtual Grindstone::WorldContextSet* Create();
		virtual void Remove(Grindstone::WorldContextSet* cxtSet);

		using WorldContextSetArray = std::vector<Grindstone::UniquePtr<Grindstone::WorldContextSet>>;

		virtual WorldContextSetArray::iterator begin();
		virtual WorldContextSetArray::const_iterator begin() const;
		virtual WorldContextSetArray::iterator end();
		virtual WorldContextSetArray::const_iterator end() const;

	private:
		size_t activeWorldIndex;
		std::map<HashedString, Grindstone::UniquePtr<Grindstone::WorldContext> (*)()> factoryFunctions;
		WorldContextSetArray worldContextSets;
	};
}
