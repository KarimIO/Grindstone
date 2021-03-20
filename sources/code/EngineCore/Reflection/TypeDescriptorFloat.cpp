#include <iostream>
#include "TypeResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Float : TypeDescriptor {
			TypeDescriptor_Float() : TypeDescriptor{ "float", sizeof(float), ReflectionTypeData::Float } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				std::cout << "float{" << *(const float*)obj << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<float>() {
			static TypeDescriptor_Float typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Float2 : TypeDescriptor {
			TypeDescriptor_Float2() : TypeDescriptor{ "Float2", sizeof(Math::Float2), ReflectionTypeData::Float2 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Float2& value = *(const Math::Float2*)obj;
				std::cout << "Float2{" << value.x << ", " << value.y << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Float2>() {
			static TypeDescriptor_Float2 typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Float3 : TypeDescriptor {
			TypeDescriptor_Float3() : TypeDescriptor{ "Float3", sizeof(Math::Float3), ReflectionTypeData::Float3 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Float3& value = *(const Math::Float3*)obj;
				std::cout << "Float3{" << value.x << ", " << value.y << ", " << value.z << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Float3>() {
			static TypeDescriptor_Float3 typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Float4 : TypeDescriptor {
			TypeDescriptor_Float4() : TypeDescriptor{ "Float4", sizeof(Math::Float4), ReflectionTypeData::Float4 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Float4& value = *(const Math::Float4*)obj;
				std::cout << "Float4{" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Float4>() {
			static TypeDescriptor_Float4 typeDesc;
			return &typeDesc;
		}
	}
}
