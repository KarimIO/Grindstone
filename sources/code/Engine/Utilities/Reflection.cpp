#include "Reflection.hpp"
#include <glm/glm.hpp>

namespace reflect {
	std::string parseStoredName(std::string v, std::string n) {
		if (v != "") return v;
		// Remove suffix _ if necessary
		if (n[n.size() - 1] == '_')
			n.erase(n.end() - 1);

		auto p = n.find_last_of('.');
		if (p != std::string::npos)
			n = n.substr(p + 1);

		return n;
	}

	std::string parseDisplayName(std::string v, std::string n) {
		if (v != "") return v;

		auto p = n.find_last_of('.');
		if (p != std::string::npos)
			n = n.substr(p + 1);

		bool first = true;
		bool symbols = false;
		for (int i = 0; i < n.size(); ++i) {
			if (!isalnum(n[i])) {
				symbols = true;
				n[i] = ' ';
			}
			else {	
				if (isalpha(n[i])) {
					if (first) {
						n[i] = toupper(n[i]);
						first = false;
					}
					else if (symbols) {
						n[i] = toupper(n[i]);
					}
					else if (isupper(n[i])) {
						n.insert(i, " ");
						++i;
					}
				}
				symbols = false;
			}
		}

		for (size_t i = n.size() - 1u; i > 0u; i--) {
			if (n[i] == ' '&&n[i] == n[i - 1]) {
				n.erase(n.begin() + i);
			}
		}

		if (n[0] == ' ') n.erase(n.begin());
		if (n[n.size() - 1] == ' ') n.erase(n.end() - 1);

		return n;
	}

	/*#define REFL_TYPE_DESC_INIT(n, t) struct TypeDescriptor_##n : TypeDescriptor { \
		TypeDescriptor_##n() : TypeDescriptor{ ##t, sizeof(t), ReflectionTypeData::Refl##t } {} \
		virtual void dump(const void* obj, int) const override { \
		std::cout << ##t << "{" << *(const n*)obj << "}"; \
		} \
	}; \
	template <> \
	TypeDescriptor* getPrimitiveDescriptor<type>() { \
		static TypeDescriptor_Int typeDesc; \
		return &typeDesc; \
	}

	REFL_TYPE_DESC_INIT(Int, int) */

	std::string stringifyMetadata(Metadata m) {
		std::string o;
		if (m & Metadata::ViewInEditor)
			o += "ViewInEditor, ";
		if (m & Metadata::SetInEditor)
			o += "SetInEditor, ";
		if (m & Metadata::ViewInScript)
			o += "ViewInScript, ";
		if (m & Metadata::SetInScript)
			o += "SetInScript, ";
		if (m & Metadata::SaveState)
			o += "SaveState, ";

		return o.substr(0, o.size() - 2);
	}

	//--------------------------------------------------------
	// A type descriptor for int
	//--------------------------------------------------------

	struct TypeDescriptor_Bool : TypeDescriptor {
		TypeDescriptor_Bool() : TypeDescriptor{ "bool", sizeof(bool), ReflectionTypeData::ReflBool } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "bool{" << *(const bool*)obj << "}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<bool>() {
		static TypeDescriptor_Bool typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for int
	//--------------------------------------------------------

	struct TypeDescriptor_Int : TypeDescriptor {
		TypeDescriptor_Int() : TypeDescriptor{ "int", sizeof(int), ReflectionTypeData::ReflInt } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "int{" << *(const int*)obj << "}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<int>() {
		static TypeDescriptor_Int typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for double
	//--------------------------------------------------------

	struct TypeDescriptor_Double : TypeDescriptor {
		TypeDescriptor_Double() : TypeDescriptor{ "double", sizeof(double), ReflectionTypeData::ReflDouble } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "double{" << *(const double*)obj << "}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<double>() {
		static TypeDescriptor_Double typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for float
	//--------------------------------------------------------

	struct TypeDescriptor_Float : TypeDescriptor {
		TypeDescriptor_Float() : TypeDescriptor{ "float", sizeof(float), ReflectionTypeData::ReflFloat } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "float{" << *(const float*)obj << "}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<float>() {
		static TypeDescriptor_Float typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for std::string
	//--------------------------------------------------------

	struct TypeDescriptor_StdString : TypeDescriptor {
		TypeDescriptor_StdString() : TypeDescriptor{ "std::string", sizeof(std::string), ReflectionTypeData::ReflString } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "std::string{\"" << *(const std::string*) obj << "\"}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<std::string>() {
		static TypeDescriptor_StdString typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for glm::vec2
	//--------------------------------------------------------

	struct TypeDescriptor_Vec2 : TypeDescriptor {
		TypeDescriptor_Vec2() : TypeDescriptor{ "glm::vec2", sizeof(glm::vec2), ReflectionTypeData::ReflVec2 } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			glm::vec2 v = *(const glm::vec2*) obj;
			std::cout << "glm::vec3{\"" << v.x << ", " << v.y << "\"}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<glm::vec2>() {
		static TypeDescriptor_Vec2 typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for glm::vec3
	//--------------------------------------------------------

	struct TypeDescriptor_Vec3 : TypeDescriptor {
		TypeDescriptor_Vec3() : TypeDescriptor{ "glm::vec3", sizeof(glm::vec3), ReflectionTypeData::ReflVec3 } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			glm::vec3 v = *(const glm::vec3*) obj;
			std::cout << "glm::vec3{\"" << v.x << ", " << v.y << ", " << v.z << "\"}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<glm::vec3>() {
		static TypeDescriptor_Vec3 typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for glm::vec4
	//--------------------------------------------------------

	struct TypeDescriptor_Vec4 : TypeDescriptor {
		TypeDescriptor_Vec4() : TypeDescriptor{ "glm::vec4", sizeof(glm::vec3), ReflectionTypeData::ReflVec4 } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			glm::vec4 v = *(const glm::vec4*) obj;
			std::cout << "glm::vec4{\"" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "\"}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<glm::vec4>() {
		static TypeDescriptor_Vec4 typeDesc;
		return &typeDesc;
	}

	//--------------------------------------------------------
	// A type descriptor for glm::quat
	//--------------------------------------------------------

	struct TypeDescriptor_Quat : TypeDescriptor {
		TypeDescriptor_Quat() : TypeDescriptor{ "glm::quat", sizeof(glm::quat), ReflectionTypeData::ReflQuat } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			glm::quat v = *(const glm::quat*) obj;
			std::cout << "glm::quat{\"" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "\"}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<glm::quat>() {
		static TypeDescriptor_Quat typeDesc;
		return &typeDesc;
	}

} // namespace reflect