#include <Common/Assert.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include "WorldContextManager.hpp"

void Grindstone::WorldContextManager::Register(HashedString name, Grindstone::UniquePtr<Grindstone::WorldContext>(*factoryFn)()) {
	factoryFunctions.emplace(name, factoryFn);

	for (Grindstone::UniquePtr<Grindstone::WorldContextSet>& newCxt : worldContextSets) {
		newCxt->Create(name, factoryFn());
	}
}

void Grindstone::WorldContextManager::Unregister(HashedString name) {
	factoryFunctions.erase(name);

	for (Grindstone::UniquePtr<Grindstone::WorldContextSet>& newCxt : worldContextSets) {
		newCxt->Remove(name);
	}
}

Grindstone::WorldContextSet* Grindstone::WorldContextManager::GetActiveWorldContextSet() {
	GS_ASSERT_ENGINE(activeWorldIndex < worldContextSets.size());

	return worldContextSets.size() > 0
		? worldContextSets[activeWorldIndex].Get()
		: nullptr;
}

void Grindstone::WorldContextManager::SetActiveWorldContextSet(Grindstone::WorldContextSet* cxtSet) {
	for (size_t i = 0; i < worldContextSets.size(); ++i) {
		UniquePtr<WorldContextSet>& cxt = worldContextSets[i];
		if (cxt.Get() == cxtSet) {
			activeWorldIndex = i;
			return;
		}
	}

	GS_ASSERT_LOG("Invalid WorldContextSet set to active!");
}

Grindstone::WorldContextSet* Grindstone::WorldContextManager::Create() {
	Grindstone::UniquePtr<Grindstone::WorldContextSet>& newCxt = worldContextSets.emplace_back(Grindstone::Memory::AllocatorCore::AllocateUnique<Grindstone::WorldContextSet>());
	newCxt->GetEntityRegistry().clear();

	for (const auto& it : factoryFunctions) {
		newCxt->Create(it.first, std::move(it.second()));
	}

	if (worldContextSets.size() == 1) {
		activeWorldIndex = 0;
	}

	return newCxt.Get();
}

void Grindstone::WorldContextManager::Remove(Grindstone::WorldContextSet* cxtSet) {
	for (size_t i = 0; i < worldContextSets.size(); ++i) {
		UniquePtr<WorldContextSet>& cxt = worldContextSets[i];
		if (cxt.Get() == cxtSet) {
			auto it = worldContextSets.begin() + i;
			worldContextSets.erase(it);

			if (activeWorldIndex >= i) {
				activeWorldIndex -= 1;
			}
			return;
		}
	}

	GS_ASSERT_LOG("Trying to remove invalid WorldContextSet!");
}

void Grindstone::WorldContextManager::ClearContextSets() {
	worldContextSets.clear();
}

Grindstone::WorldContextManager::WorldContextSetArray::iterator Grindstone::WorldContextManager::begin() {
	return worldContextSets.begin();
}

Grindstone::WorldContextManager::WorldContextSetArray::const_iterator Grindstone::WorldContextManager::begin() const {
	return worldContextSets.begin();
}

Grindstone::WorldContextManager::WorldContextSetArray::iterator Grindstone::WorldContextManager::end() {
	return worldContextSets.end();
}

Grindstone::WorldContextManager::WorldContextSetArray::const_iterator Grindstone::WorldContextManager::end() const {
	return worldContextSets.end();
}

