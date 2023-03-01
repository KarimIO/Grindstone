#pragma once

#include "TypeDescriptor.hpp"
#include "TypeResolver.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_AssetReference : TypeDescriptor {
			std::string name;
			AssetType assetType;

			template <typename ItemType>
			TypeDescriptor_AssetReference(ItemType*)
				: TypeDescriptor {
					"AssetReference<>",
					sizeof(Grindstone::GenericAssetReference),
					ReflectionTypeData::AssetReference
				},
				assetType(ItemType::GetStaticType()),
				name((std::string("AssetReference<") + ItemType::GetAssetTypeName() + ">").c_str())
			{}

			virtual const char* GetFullName() const override {
				return name.c_str();
			}
		};

		template <typename T>
		class TypeResolver<Grindstone::AssetReference<T>> {
		public:
			static TypeDescriptor* Get() {
				static TypeDescriptor_AssetReference typeDesc{ (T*) nullptr };
				return &typeDesc;
			}
		};
	}
}
