#pragma once

#include <string>
#include <type_traits>

#include <EngineCore/Assets/Asset.hpp>

namespace Grindstone::AssetFunctions {
	[[nodiscard]] void* Get(Grindstone::AssetType assetType, Grindstone::Uuid uuid);
	[[nodiscard]] void* GetAndIncrement(Grindstone::AssetType assetType, Grindstone::Uuid uuid);
	void Increment(Grindstone::AssetType assetType, Grindstone::Uuid uuid);
	void Decrement(Grindstone::AssetType assetType, Grindstone::Uuid uuid);
}

namespace Grindstone {
	struct GenericAssetReference {
		GenericAssetReference() = default;
		GenericAssetReference(Grindstone::Uuid uuid) : uuid(uuid) {}
		GenericAssetReference(const GenericAssetReference& other) = default;
		GenericAssetReference(GenericAssetReference&& other) = default;

		GenericAssetReference& operator=(const GenericAssetReference& other) noexcept {
			uuid = other.uuid;
			return *this;
		}

		GenericAssetReference& operator=(GenericAssetReference&& other) noexcept {
			uuid = other.uuid;
			return *this;
		}

		operator bool() const noexcept {
			return uuid.IsValid();
		}

		bool IsValid() const noexcept {
			return uuid.IsValid();
		}

		Uuid uuid;
	};

	template<typename T>
	struct AssetReference : public GenericAssetReference {
		static_assert(std::is_base_of_v<Grindstone::Asset, T>, "T not derived from Grindstone::Asset");

		static AssetReference<T> CreateWithoutIncrement(Uuid uuid) {
			return AssetReference(uuid);
		}

		static AssetReference<T> CreateAndIncrement(Uuid uuid) {
			Grindstone::AssetFunctions::Increment(T::GetStaticType(), uuid);
			return AssetReference(uuid);
		}

		AssetReference() : GenericAssetReference() {}

		AssetReference(const AssetReference& other) : GenericAssetReference(other.uuid) {
			Grindstone::AssetFunctions::Increment(T::GetStaticType(), uuid);
		}

		AssetReference(AssetReference&& other) : GenericAssetReference(other.uuid) {}

		AssetReference& operator=(const AssetReference& other) noexcept {
			if (uuid != other.uuid) {
				if (uuid.IsValid()) {
					Grindstone::AssetFunctions::Decrement(T::GetStaticType(), uuid);
				}

				uuid = other.uuid;
				Grindstone::AssetFunctions::Increment(T::GetStaticType(), uuid);
			}

			return *this;
		}

		AssetReference& operator=(AssetReference&& other) noexcept {
			if (uuid != other.uuid) {
				if (uuid.IsValid()) {
					Grindstone::AssetFunctions::Decrement(T::GetStaticType(), uuid);
				}

				uuid = other.uuid;
			}

			return *this;
		}

		~AssetReference() {
			Grindstone::AssetFunctions::Decrement(T::GetStaticType(), uuid);
		}

		void Release() {
			Grindstone::AssetFunctions::Decrement(T::GetStaticType(), uuid);
			uuid = Uuid();
		}

		[[nodiscard]] T* Get() {
			return reinterpret_cast<T*>(Grindstone::AssetFunctions::Get(T::GetStaticType(), uuid));
		}

		[[nodiscard]] const T* Get() const {
			return reinterpret_cast<T*>(Grindstone::AssetFunctions::Get(T::GetStaticType(), uuid));
		}

		// Don't implement operators * or ->, as they imply getting data is a cheap operator.
		// It's relatively cheap, but best practice is to cache the data when you use it
		// within the function it's in, but not across multiple frames.

	private:
		AssetReference(Grindstone::Uuid uuid) : GenericAssetReference(uuid) {}
	};
}
