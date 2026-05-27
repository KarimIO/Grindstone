#pragma once

#include <array>
#include "TypeDescriptor.hpp"
#include "TypeResolver.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_FixedArray : TypeDescriptor {
		std::string name;
		TypeDescriptor* itemType;
		size_t size;
		void* (*getItem)(const void*, size_t);
		void (*emplaceBack)(void*);

		template <typename ItemType, size_t N>
		TypeDescriptor_FixedArray(ItemType(*)[N])
			: TypeDescriptor{ "FixedArray<>", sizeof(ItemType) * N, ReflectionTypeData::FixedArray },
			itemType{ TypeResolver<ItemType>::Get() },
			name{ (std::string(TypeResolver<ItemType>::Get()->GetFullName()) + "[" + std::to_string(N) + "]").c_str()},
			size(N)
		{
			getItem = [](const void* arrayPtr, size_t index) -> void* {
				ItemType* vec = (ItemType*)(arrayPtr);
				return (void*)&vec[index];
			};
		}

		virtual const char* GetFullName() const override {
			return name.c_str();
		}
	};

	template <typename T, size_t N>
	class TypeResolver<T[N]> {
	public:
		static TypeDescriptor* Get() {
			static TypeDescriptor_FixedArray typeDesc( (T(*)[N])nullptr );
			return &typeDesc;
		}
	};
}
