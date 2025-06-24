#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include "AssetReference.hpp"

void* Grindstone::AssetFunctions::Get(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	return Grindstone::EngineCore::GetInstance().assetManager->GetAssetByUuid(assetType, uuid);
}

void* Grindstone::AssetFunctions::GetAndIncrement(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	return Grindstone::EngineCore::GetInstance().assetManager->GetAndIncrementAssetCount(assetType, uuid);
}

void Grindstone::AssetFunctions::Increment(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	Grindstone::EngineCore::GetInstance().assetManager->IncrementAssetCount(assetType, uuid);
}

void Grindstone::AssetFunctions::Decrement(Grindstone::AssetType assetType, Grindstone::Uuid uuid) {
	Grindstone::EngineCore::GetInstance().assetManager->DecrementAssetCount(assetType, uuid);
}
