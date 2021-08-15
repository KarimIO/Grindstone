#include <iostream>
#include <vector>
#include "TypeDescriptor.hpp"
#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_StdVector : TypeDescriptor {
			std::string name;
			TypeDescriptor* itemType;
			size_t(*getSize)(const void*);
			const void* (*getItem)(const void*, size_t);

			template <typename ItemType>
			TypeDescriptor_StdVector(ItemType*)
				: TypeDescriptor{ "std::vector<>", sizeof(std::vector<ItemType>), ReflectionTypeData::ReflVector },
				name{ (std::string("std::vector<") + itemType->getFullName() + ">").c_str() },
				itemType{ TypeResolver<ItemType>::get() } {
				getSize = [](const void* vecPtr) -> size_t {
					const auto& vec = *(const std::vector<ItemType>*) vecPtr;
					return vec.size();
				};
				getItem = [](const void* vecPtr, size_t index) -> const void* {
					const auto& vec = *(const std::vector<ItemType>*) vecPtr;
					return &vec[index];
				};
			}

			virtual const char* getFullName() const override {
				return name.c_str();
			}
		};

		template <typename T>
		class TypeResolver<std::vector<T>> {
		public:
			static TypeDescriptor* getPrimitiveDescriptor() {
				static TypeDescriptor_StdVector typeDesc{ (T*) nullptr };
				return &typeDesc;
			}
		};

		template <typename T>
		TypeDescriptor* getPrimitiveDescriptor<std::vector<T>>() {
			static TypeDescriptor_StdVector typeDesc{ (T*) nullptr };
			return &typeDesc;
		}
	}
}
