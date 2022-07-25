#pragma once

#include <string>
#include "TypeDescriptor.hpp"

namespace Grindstone {
	namespace Reflection {
		template <typename T>
		struct TypeResolver {
			static TypeDescriptor* Get() {
				return DefaultResolver::Get<T>();
			}
		};
	}
}
