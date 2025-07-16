#include <Common/Assert.hpp>
#include "WorldContextManager.hpp"

void Grindstone::WorldContextManager::Register(HashedString name, Grindstone::UniquePtr<Grindstone::WorldContext>(*factoryFn)()) {
	factoryFunctions.emplace(name, factoryFn);

	for (Grindstone::WorldContextSet& newCxt : worldContextSets) {
		newCxt.Create(name, factoryFn());
	}
}

void Grindstone::WorldContextManager::Unregister(HashedString name) {
	factoryFunctions.erase(name);

	for (Grindstone::WorldContextSet& newCxt : worldContextSets) {
		newCxt.Remove(name);
	}
}

Grindstone::WorldContextSet* Grindstone::WorldContextManager::GetActiveWorldContextSet() {
	GS_ASSERT_ENGINE(activeWorldIndex < worldContextSets.size());

	return worldContextSets.size() > 0
		? &worldContextSets[activeWorldIndex]
		: nullptr;
}

void Grindstone::WorldContextManager::SetActiveWorldContextSet(Grindstone::WorldContextSet* cxtSet) {
	GS_ASSERT_ENGINE(cxtSet >= worldContextSets.data() && cxtSet < worldContextSets.data() + worldContextSets.size());

	auto it = worldContextSets.begin() + (cxtSet - &worldContextSets[0]);
	size_t index = std::distance(worldContextSets.begin(), it);

	activeWorldIndex = index;
}

Grindstone::WorldContextSet& Grindstone::WorldContextManager::Create() {
	Grindstone::WorldContextSet& newCxt = worldContextSets.emplace_back();
	newCxt.GetEntityRegistry().clear();

	for (const auto& it : factoryFunctions) {
		newCxt.Create(it.first, std::move(it.second()));
	}

	if (worldContextSets.size() == 1) {
		activeWorldIndex = 0;
	}

	return newCxt;
}

void Grindstone::WorldContextManager::Remove(Grindstone::WorldContextSet& cxtSet) {
	GS_ASSERT_ENGINE(&cxtSet >= worldContextSets.data() && &cxtSet < worldContextSets.data() + worldContextSets.size());

	auto it = worldContextSets.begin() + (&cxtSet - &worldContextSets[0]);
	size_t index = std::distance(worldContextSets.begin(), it);
	worldContextSets.erase(it);

	if (activeWorldIndex >= index) {
		activeWorldIndex -= 1;
	}
}
