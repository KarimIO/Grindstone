#pragma once

#include <vector>
#include "TypeDescriptor.hpp"
#include "Metadata.hpp"
#include "PrintReflectionData.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Struct : TypeDescriptor {
		struct Member {
			std::string variableName;
			std::string displayName;
			std::string storedName;
			size_t offset;

			Metadata metadata;

			TypeDescriptor* type;

			void (*onChangeCallback)(void *owner);
		};
			
		struct Category {
			std::string name;
			std::vector<Member> members;
			std::vector<Category> categories;
		};

		Category category;

		TypeDescriptor_Struct() = default;

		TypeDescriptor_Struct(
			void(*init)(TypeDescriptor_Struct*)
		) : TypeDescriptor{ nullptr, 0, ReflectionTypeData::Struct } {
			init(this);
		}

		TypeDescriptor_Struct(
			const char* name,
			size_t size,
			const Category &init
		) : TypeDescriptor{ nullptr, 0, ReflectionTypeData::Struct }, category{ init } {}
	};
}
