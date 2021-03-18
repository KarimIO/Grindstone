#include <iostream>
#include <vector>
#include "TypeDescriptor.hpp"
#include "Metadata.hpp"
#include "PrintReflectionData.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Struct : TypeDescriptor {
			struct Member {
				std::string variable_name;
				std::string display_name;
				std::string stored_name;
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

			TypeDescriptor_Struct(
				void(*init)(TypeDescriptor_Struct*)
			) : TypeDescriptor{ nullptr, 0, ReflectionTypeData::ReflStruct } {
				init(this);
			}

			TypeDescriptor_Struct(
				const char* name,
				size_t size,
				const Category &init
			) : TypeDescriptor{ nullptr, 0, ReflectionTypeData::ReflStruct }, category{ init } {}

			virtual void dump(const void* obj, int indentLevel) const override {
				std::cout << name << " {" << std::endl;
				for (const Member& member : category.members) {
					std::cout << std::string(4 * (indentLevel + 1), ' ') << member.display_name;
					if (member.metadata != Metadata::NoMetadata)
						std::cout << " [" << stringifyMetadata(member.metadata) << "]";
					std::cout << " = ";
					member.type->dump((char*)obj + member.offset, indentLevel + 1);
					std::cout << std::endl;
				}
				std::cout << std::string(4 * indentLevel, ' ') << "}";

			}
		};
	}
}
