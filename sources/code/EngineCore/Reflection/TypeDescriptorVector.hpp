#pragma once

#include <vector>
#include "TypeDescriptor.hpp"
#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_StdVector : TypeDescriptor {
			std::string name;
			TypeDescriptor* itemType;
			size_t (*getSize)(const void*);
			void* (*getItem)(const void*, size_t);
			void (*erase)(void*, size_t indexs);
			void (*emplaceBack)(void*);

			template <typename ItemType>
			TypeDescriptor_StdVector(ItemType*)
				: TypeDescriptor{ "std::vector<>", sizeof(std::vector<ItemType>), ReflectionTypeData::Vector },
				itemType{ TypeResolver<ItemType>::Get()},
				name{ (std::string("std::vector<") + TypeResolver<ItemType>::Get()->GetFullName() + ">").c_str() }
			{
				getSize = [](const void* vecPtr) -> size_t {
					const auto& vec = *(const std::vector<ItemType>*) vecPtr;
					return vec.size();
				};

				getItem = [](const void* vecPtr, size_t index) -> const void* {
					const auto& vec = *(const std::vector<ItemType>*) vecPtr;
					return (void*)&vec[index];
				};

				erase = [](void* vecPtr, size_t index) -> void {
					auto& vec = *(std::vector<ItemType>*) vecPtr;
					vec.erase(vec.begin() + index);
				};

				emplaceBack = [](void* vecPtr) -> void {
					auto& vec = *(std::vector<ItemType>*) vecPtr;
					vec.emplace_back();
				};
			}

			virtual const char* GetFullName() const override {
				return name.c_str();
			}
		};

		template <typename T>
		class TypeResolver<std::vector<T>> {
		public:
			static TypeDescriptor* Get() {
				static TypeDescriptor_StdVector typeDesc{ (T*) nullptr };
				return &typeDesc;
			}
		};
	}
}
