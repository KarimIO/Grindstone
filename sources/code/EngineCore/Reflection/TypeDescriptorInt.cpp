#include <iostream>
#include "TypeResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Int : TypeDescriptor {
			TypeDescriptor_Int() : TypeDescriptor{ "Int", sizeof(int), ReflectionTypeData::Int } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				std::cout << "Int{" << *(const int*)obj << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<int>() {
			static TypeDescriptor_Int typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Int2 : TypeDescriptor {
			TypeDescriptor_Int2() : TypeDescriptor{ "Int2", sizeof(Math::Int2), ReflectionTypeData::Int2 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Int2& value = *(const Math::Int2*)obj;
				std::cout << "Int2{" << value.x << ", " << value.y << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Int2>() {
			static TypeDescriptor_Int2 typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Int3 : TypeDescriptor {
			TypeDescriptor_Int3() : TypeDescriptor{ "Int3", sizeof(Math::Int3), ReflectionTypeData::Int4 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Int3& value = *(const Math::Int3*)obj;
				std::cout << "Int3{" << value.x << ", " << value.y << ", " << value.z << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Int3>() {
			static TypeDescriptor_Int3 typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Int4 : TypeDescriptor {
			TypeDescriptor_Int4() : TypeDescriptor{ "Int4", sizeof(Math::Int4), ReflectionTypeData::Int4 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Int4& value = *(const Math::Int4*)obj;
				std::cout << "Int4{" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Int4>() {
			static TypeDescriptor_Int4 typeDesc;
			return &typeDesc;
		}
	}
}
