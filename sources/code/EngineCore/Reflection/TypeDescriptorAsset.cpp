#include "DefaultResolver.hpp"
#include "EngineCore/Assets/AssetFile.hpp"

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
				sizeof(MeshReference),
				ReflectionTypeData::AssetReference
			}, assetType(assetType), loaderFn(loaderFn) {}

			AssetType assetType;
			void(*loaderFn)(Uuid uuid);

			void LoadFile(Uuid& uuid) {
				loaderFn(uuid);
			}
		};

		struct TypeDescriptor_MeshReference : public TypeDescriptor_AssetReference {
			TypeDescriptor_MeshReference() :
			TypeDescriptor_AssetReference(
				AssetType::Mesh,
				"Mesh Reference",
				sizeof(MeshReference)
			) {}
		};

		template <>
		TypeDescriptor* GetPrimitiveDescriptor<MeshReference>() {
			static TypeDescriptor_MeshReference typeDesc;
			return &typeDesc;
		}
	}
}
