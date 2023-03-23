#pragma once

#include <string>

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor {
			const char* name;
			size_t size;

			enum class ReflectionTypeData : char {
				Struct = 0,
				Vector,
				String,
				Bool,
				Int,
				Int2,
				Int3,
				Int4,
				Float,
				Float2,
				Float3,
				Float4,
				Double,
				Double2,
				Double3,
				Double4,
				Quaternion,
				AssetReference
			};
			ReflectionTypeData type;

			TypeDescriptor() = default;
			TypeDescriptor(const char* name, size_t size, ReflectionTypeData t) : name{ name }, size{ size }, type{ t } {}
			virtual ~TypeDescriptor() {}
			virtual const char* GetFullName() const { return name; }
		};
	}
}
