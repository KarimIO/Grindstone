#include "DefaultResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Char : TypeDescriptor {
		TypeDescriptor_Char() : TypeDescriptor{ "Char", sizeof(char), ReflectionTypeData::Char } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<char>() {
		static TypeDescriptor_Char typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uchar : TypeDescriptor {
		TypeDescriptor_Uchar() : TypeDescriptor{ "Uchar", sizeof(unsigned char), ReflectionTypeData::Uchar } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<unsigned char>() {
		static TypeDescriptor_Uchar typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Short : TypeDescriptor {
		TypeDescriptor_Short() : TypeDescriptor{ "Short", sizeof(short), ReflectionTypeData::Short } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<short>() {
		static TypeDescriptor_Short typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Ushort : TypeDescriptor {
		TypeDescriptor_Ushort() : TypeDescriptor{ "Ushort", sizeof(unsigned short), ReflectionTypeData::Ushort } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<unsigned short>() {
		static TypeDescriptor_Ushort typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int : TypeDescriptor {
		TypeDescriptor_Int() : TypeDescriptor{ "Int", sizeof(int), ReflectionTypeData::Int } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<int>() {
		static TypeDescriptor_Int typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int2 : TypeDescriptor {
		TypeDescriptor_Int2() : TypeDescriptor{ "Int2", sizeof(Math::Int2), ReflectionTypeData::Int2 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Int2>() {
		static TypeDescriptor_Int2 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int3 : TypeDescriptor {
		TypeDescriptor_Int3() : TypeDescriptor{ "Int3", sizeof(Math::Int3), ReflectionTypeData::Int3 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Int3>() {
		static TypeDescriptor_Int3 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int4 : TypeDescriptor {
		TypeDescriptor_Int4() : TypeDescriptor{ "Int4", sizeof(Math::Int4), ReflectionTypeData::Int4 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Int4>() {
		static TypeDescriptor_Int4 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint2 : TypeDescriptor {
		TypeDescriptor_Uint2() : TypeDescriptor{ "Uint2", sizeof(Math::Uint2), ReflectionTypeData::Uint2 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Uint2>() {
		static TypeDescriptor_Uint2 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint3 : TypeDescriptor {
		TypeDescriptor_Uint3() : TypeDescriptor{ "Uint3", sizeof(Math::Uint3), ReflectionTypeData::Uint3 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Uint3>() {
		static TypeDescriptor_Uint3 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint4 : TypeDescriptor {
		TypeDescriptor_Uint4() : TypeDescriptor{ "Uint4", sizeof(Math::Uint4), ReflectionTypeData::Uint4 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Uint4>() {
		static TypeDescriptor_Uint4 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int8 : TypeDescriptor {
		TypeDescriptor_Int8() : TypeDescriptor{ "int8_t", sizeof(int8_t), ReflectionTypeData::Int8 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<int8_t>() {
		static TypeDescriptor_Int8 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int16 : TypeDescriptor {
		TypeDescriptor_Int16() : TypeDescriptor{ "int16_t", sizeof(int16_t), ReflectionTypeData::Int16 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<int16_t>() {
		static TypeDescriptor_Int16 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int32 : TypeDescriptor {
		TypeDescriptor_Int32() : TypeDescriptor{ "int32_t", sizeof(int32_t), ReflectionTypeData::Int32 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<int32_t>() {
		static TypeDescriptor_Int32 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int64 : TypeDescriptor {
		TypeDescriptor_Int64() : TypeDescriptor{ "int64_t", sizeof(int64_t), ReflectionTypeData::Int64 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<int64_t>() {
		static TypeDescriptor_Int64 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint8 : TypeDescriptor {
		TypeDescriptor_Uint8() : TypeDescriptor{ "uint8_t", sizeof(uint8_t), ReflectionTypeData::Uint8 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<uint8_t>() {
		static TypeDescriptor_Uint8 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint16 : TypeDescriptor {
		TypeDescriptor_Uint16() : TypeDescriptor{ "uint16_t", sizeof(uint16_t), ReflectionTypeData::Uint16 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<uint16_t>() {
		static TypeDescriptor_Uint16 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint32 : TypeDescriptor {
		TypeDescriptor_Uint32() : TypeDescriptor{ "uint32_t", sizeof(uint32_t), ReflectionTypeData::Uint32 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<uint32_t>() {
		static TypeDescriptor_Uint32 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Uint64 : TypeDescriptor {
		TypeDescriptor_Uint64() : TypeDescriptor{ "uint64_t", sizeof(uint64_t), ReflectionTypeData::Uint64 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<uint64_t>() {
		static TypeDescriptor_Uint64 typeDesc;
		return &typeDesc;
	}
}
