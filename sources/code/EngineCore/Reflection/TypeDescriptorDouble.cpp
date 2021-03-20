#include <iostream>
#include "TypeResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Double : TypeDescriptor {
			TypeDescriptor_Double() : TypeDescriptor{ "Double", sizeof(double), ReflectionTypeData::Double } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				std::cout << "Double{" << *(const double*)obj << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<double>() {
			static TypeDescriptor_Double typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Double2 : TypeDescriptor {
			TypeDescriptor_Double2() : TypeDescriptor{ "Double2", sizeof(Math::Double2), ReflectionTypeData::Double2 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Double2& value = *(const Math::Double2*)obj;
				std::cout << "Double2{" << value.x << ", " << value.y << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Double2>() {
			static TypeDescriptor_Double2 typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Double3 : TypeDescriptor {
			TypeDescriptor_Double3() : TypeDescriptor{ "Double3", sizeof(Math::Double3), ReflectionTypeData::Double4 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Double3& value = *(const Math::Double3*)obj;
				std::cout << "Double3{" << value.x << ", " << value.y << ", " << value.z << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Double3>() {
			static TypeDescriptor_Double3 typeDesc;
			return &typeDesc;
		}

		struct TypeDescriptor_Double4 : TypeDescriptor {
			TypeDescriptor_Double4() : TypeDescriptor{ "Double4", sizeof(Math::Double4), ReflectionTypeData::Double4 } {}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Double4& value = *(const Math::Double4*)obj;
				std::cout << "Double4{" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Double4>() {
			static TypeDescriptor_Double4 typeDesc;
			return &typeDesc;
		}
	}
}
