#ifndef _REFLECTION_HPP
#define _REFLECTION_HPP

namespace reflect {

	std::string parseDisplayName(std::string v, std::string n);
	std::string parseStoredName(std::string v, std::string n);

	//--------------------------------------------------------
	// Base class of all type descriptors
	//--------------------------------------------------------

	struct TypeDescriptor {
		const char* name;
		size_t size;

		enum ReflectionTypeData : char {
			ReflStruct = 0,
			ReflVector,
			ReflString,
			ReflBool,
			ReflInt,
			ReflFloat,
			ReflDouble,
			ReflVec2,
			ReflVec3,
			ReflVec4,
			ReflQuat,
		};
		ReflectionTypeData type;

		TypeDescriptor(const char* name, size_t size, ReflectionTypeData t) : name{ name }, size{ size }, type{ t } {}
		virtual ~TypeDescriptor() {}
		virtual std::string getFullName() const { return name; }
		virtual void dump(const void* obj, int indentLevel = 0) const = 0;
	};

	//--------------------------------------------------------
	// Finding type descriptors
	//--------------------------------------------------------

	// Declare the function template that handles primitive types such as int, std::string, etc.:
	template <typename T>
	TypeDescriptor* getPrimitiveDescriptor();

	// A helper class to find TypeDescriptors in different ways:
	struct DefaultResolver {
		template <typename T> static char func(decltype(&T::Reflection));
		template <typename T> static int func(...);
		template <typename T>
		struct IsReflected {
			enum { value = (sizeof(func<T>(nullptr)) == sizeof(char)) };
		};

		// This version is called if T has a static member named "Reflection":
		template <typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
		static TypeDescriptor* get() {
			return &T::Reflection;
		}

		// This version is called otherwise:
		template <typename T, typename std::enable_if<!IsReflected<T>::value, int>::type = 0>
		static TypeDescriptor* get() {
			return getPrimitiveDescriptor<T>();
		}
	};

	// This is the primary class template for finding all TypeDescriptors:
	template <typename T>
	struct TypeResolver {
		static TypeDescriptor* get() {
			return DefaultResolver::get<T>();
		}
	};

	enum Metadata : short {
		NoMetadata = 0,
		ViewInEditor = 1 << 0,
		SetInEditor = 1 << 1,
		ViewInScript = 1 << 2,
		SetInScript = 1 << 3,
		ViewInAll = ViewInEditor | ViewInScript,
		SetInAll = ViewInAll | SetInEditor | SetInScript,
		SaveState = 1 << 4,
		SaveSetAndView = SetInAll | SaveState
	};
	
	std::string stringifyMetadata(Metadata m);

	//--------------------------------------------------------
	// Type descriptors for user-defined structs/classes
	//--------------------------------------------------------

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

		TypeDescriptor_Struct(void(*init)(TypeDescriptor_Struct*)) : TypeDescriptor{ nullptr, 0, ReflectionTypeData::ReflStruct } {
			init(this);
		}
		TypeDescriptor_Struct(const char* name, size_t size, const Category &init) : TypeDescriptor{ nullptr, 0, ReflectionTypeData::ReflStruct }, category{ init } {
		}
		virtual void dump(const void* obj, int indentLevel) const override {
			/*std::cout << name << " {" << std::endl;
			for (const Member& member : members) {
				std::cout << std::string(4 * (indentLevel + 1), ' ') << member.name;
				if (member.metadata != Metadata::None)
					std::cout << " [" << stringifyMetadata(member.metadata) << "]";
				std::cout << " = ";
				member.type->dump((char*)obj + member.offset, indentLevel + 1);
				std::cout << std::endl;
			}
			std::cout << std::string(4 * indentLevel, ' ') << "}";
			*/
		}
	};

// #define REFLECT_MOD(x, ...) reflect::x | REFLECT_MOD(_VAR_ARGS_)

#define REFLECT_SYSTEM() \
public: \
    static reflect::TypeDescriptor_Struct grindstone_reflection_info_; \
	virtual reflect::TypeDescriptor_Struct *getReflection() { return &grindstone_reflection_info_; } \
	const static ComponentType static_system_type_;

#define REFLECT() \
public: \
    friend struct reflect::DefaultResolver; \
    static reflect::TypeDescriptor_Struct grindstone_reflection_info_; \
    static void initReflection(reflect::TypeDescriptor_Struct*);

#define REFLECT_STRUCT_BEGIN(type, system, enum_type) \
const ComponentType system::static_system_type_ = enum_type; \
    reflect::TypeDescriptor_Struct type::grindstone_reflection_info_{type::initReflection}; \
    reflect::TypeDescriptor_Struct system::grindstone_reflection_info_ = type::grindstone_reflection_info_; \
    void type::initReflection(reflect::TypeDescriptor_Struct* typeDesc) { \
        using T = type; \
        typeDesc->name = #type; \
        typeDesc->size = sizeof(T); \
        typeDesc->category = { "", {

#define REFLECT_STRUCT_MEMBER_D(name, display_name, stored_name, mods, callback) \
            {#name, reflect::parseDisplayName(display_name, #name), reflect::parseStoredName(stored_name, #name), offsetof(T, name), mods, reflect::TypeResolver<decltype(T::name)>::get(), callback},

#define REFLECT_STRUCT_MEMBER(name) REFLECT_STRUCT_MEMBER_D(name, "", "", reflect::Metadata::SaveSetAndView, nullptr)

#define REFLECT_NO_SUBCAT() }, {}
#define REFLECT_SUBCATS_START() }, {
#define REFLECT_SUBCATS_END() }}

#define REFLECT_SUBCAT_START(name) { name, {
#define REFLECT_SUBCAT_END() },

#define REFLECT_STRUCT_END() }; \
    }

	//--------------------------------------------------------
	// Type descriptors for std::vector
	//--------------------------------------------------------

	struct TypeDescriptor_StdVector : TypeDescriptor {
		TypeDescriptor* itemType;
		size_t(*getSize)(const void*);
		const void* (*getItem)(const void*, size_t);

		template <typename ItemType>
		TypeDescriptor_StdVector(ItemType*)
			: TypeDescriptor{ "std::vector<>", sizeof(std::vector<ItemType>), ReflectionTypeData::ReflVector },
			itemType{ TypeResolver<ItemType>::get() } {
			getSize = [](const void* vecPtr) -> size_t {
				const auto& vec = *(const std::vector<ItemType>*) vecPtr;
				return vec.size();
			};
			getItem = [](const void* vecPtr, size_t index) -> const void* {
				const auto& vec = *(const std::vector<ItemType>*) vecPtr;
				return &vec[index];
			};
		}
		virtual std::string getFullName() const override {
			return std::string("std::vector<") + itemType->getFullName() + ">";
		}
		virtual void dump(const void* obj, int indentLevel) const override {
			size_t numItems = getSize(obj);
			std::cout << getFullName();
			if (numItems == 0) {
				std::cout << "{}";
			}
			else {
				std::cout << "{" << std::endl;
				for (size_t index = 0; index < numItems; index++) {
					std::cout << std::string(4 * (indentLevel + 1), ' ') << "[" << index << "] ";
					itemType->dump(getItem(obj, index), indentLevel + 1);
					std::cout << std::endl;
				}
				std::cout << std::string(4 * indentLevel, ' ') << "}";
			}
		}
	};

	// Partially specialize TypeResolver<> for std::vectors:
	template <typename T>
	class TypeResolver<std::vector<T>> {
	public:
		static TypeDescriptor* get() {
			static TypeDescriptor_StdVector typeDesc{ (T*) nullptr };
			return &typeDesc;
		}
	};

} // namespace reflect

#endif