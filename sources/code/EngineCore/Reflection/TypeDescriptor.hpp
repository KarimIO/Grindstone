#pragma once

#include <string>

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor {
			const char* name;
			size_t size;

			enum class ReflectionTypeData : char {
				ReflStruct = 0,
				ReflVector,
				ReflString,
				ReflBool,
				ReflInt,
				ReflFloat,
				ReflDouble
			};
			ReflectionTypeData type;

			TypeDescriptor(const char* name, size_t size, ReflectionTypeData t) : name{ name }, size{ size }, type{ t } {}
			virtual ~TypeDescriptor() {}
			virtual std::string getFullName() const { return name; }
			virtual void dump(const void* obj, int indentLevel = 0) const = 0;
		};
	}
}
