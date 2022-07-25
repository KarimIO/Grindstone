#pragma once

#include <string>
#include "TypeDescriptor.hpp"

namespace Grindstone {
	namespace Reflection {
		template <typename T>
		TypeDescriptor* GetPrimitiveDescriptor();

		struct DefaultResolver {
			template <typename T> static char func(decltype(&T::Reflection));
			template <typename T> static int func(...);
			template <typename T>
			struct IsReflected {
				enum { value = (sizeof(func<T>(nullptr)) == sizeof(char)) };
			};

			// This version is called if T has a static member named "Reflection":
			template <typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
			static TypeDescriptor* Get() {
				return &T::Reflection;
			}

			// This version is called otherwise:
			template <typename T, typename std::enable_if<!IsReflected<T>::value, int>::type = 0>
			static TypeDescriptor* Get() {
				return GetPrimitiveDescriptor<T>();
			}
		};
	}
}
