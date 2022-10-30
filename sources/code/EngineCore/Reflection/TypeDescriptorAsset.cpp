#include "DefaultResolver.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_AssetReference : TypeDescriptor {
			TypeDescriptor_AssetReference(
				AssetType assetType,
				const char* name,
				size_t size,
				void(*loaderFn)(Uuid uuid)
			) :
			TypeDescriptor{
				"Asset Reference",
				sizeof(int), // TODO: Change to sizeof the AssetReference
				ReflectionTypeData::AssetReference
			}, assetType(assetType), loaderFn(loaderFn) {}

			AssetType assetType;
			void(*loaderFn)(Uuid uuid);

			void LoadFile(Uuid& uuid) {
				loaderFn(uuid);
			}
		};
	}
}
