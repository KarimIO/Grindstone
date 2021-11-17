#pragma once

#include <string>
#include "TypeDescriptor.hpp"

namespace Grindstone {
	namespace Reflection {
		template <typename T>
		struct TypeResolver {
			static TypeDescriptor* get() {
				return DefaultResolver::get<T>();
			}
		};
	}
}
