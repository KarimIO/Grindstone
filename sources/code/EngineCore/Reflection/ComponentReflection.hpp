#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "TypeResolver.hpp"
#include "DefaultResolver.hpp"
#include "TypeDescriptorStruct.hpp"
#include "TypeDescriptorVector.hpp"
#include "PrintReflectionData.hpp"

namespace Grindstone {
	namespace Reflection {
#define REFLECT(name) \
	public: \
		friend struct Grindstone::Reflection::DefaultResolver; \
		static Grindstone::Reflection::TypeDescriptor_Struct reflectionInfo; \
		static void InitializeReflection(Grindstone::Reflection::TypeDescriptor_Struct*); \
		static const char* GetComponentName() { return name;  };

#define REFLECT_STRUCT_BEGIN(type) \
		Grindstone::Reflection::TypeDescriptor_Struct type::reflectionInfo{type::InitializeReflection}; \
		void type::InitializeReflection(Grindstone::Reflection::TypeDescriptor_Struct* typeDesc) { \
			using T = type; \
			typeDesc->name = #type; \
			typeDesc->size = sizeof(T); \
			typeDesc->category = { "", {

#define REFLECT_STRUCT_MEMBER_D(name, displayName, storedName, mods, callback) \
			{#name, Grindstone::Reflection::ParseDisplayName(displayName, #name), Grindstone::Reflection::ParseStoredName(storedName, #name), offsetof(T, name), mods, Grindstone::Reflection::TypeResolver<decltype(T::name)>::get(), callback},

#define REFLECT_STRUCT_MEMBER(name) REFLECT_STRUCT_MEMBER_D(name, "", "", Grindstone::Reflection::Metadata::SaveSetAndView, nullptr)

#define REFLECT_NO_SUBCAT() }, {}
#define REFLECT_SUBCATS_START() }, {
#define REFLECT_SUBCATS_END() }}

#define REFLECT_SUBCAT_START(name) { name, {
#define REFLECT_SUBCAT_END() },

#define REFLECT_STRUCT_END() }; \
		}

	}
}
