#include "DefaultResolver.hpp"
#include "EngineCore/Assets/AssetFile.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_AssetReference : TypeDescriptor {
			enum class AssetType {
				Text = 0,
				Mesh,
				Material,
				Texture,
				Audio,
			};

			TypeDescriptor_AssetReference(
				AssetType assetType,
				const char* name,
				size_t size
			) :
			TypeDescriptor{
				"Asset Reference",
				sizeof(MeshReference),
				ReflectionTypeData::AssetReference
			}, assetType(assetType) {}

			AssetType assetType;
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
		TypeDescriptor* getPrimitiveDescriptor<MeshReference>() {
			static TypeDescriptor_MeshReference typeDesc;
			return &typeDesc;
		}
	}
}
